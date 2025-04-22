using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Types;
using System.Diagnostics;

namespace Compiler.AstBuilding;

internal class ConvertBuilder(AstBuilderContext context, DefaultValueExpressionResolver defaultValueExpressionResolver)
{
  // Returns null if the assignment cannot be performed and an error should be reported. If the expression's data type is already identical to the target data
  // type, the expression is simply returned without modification.
  public ExpressionAstNode? TryBuildAssign(
    SourceLocation sourceLocation,
    ScopeAstNode scope,
    ScopeTracker scopeTracker,
    ExpressionAstNode expression,
    AstDataType toDataType)
  {
    if (expression.DataType.IsError || toDataType.IsError || expression.DataType.IsIdenticalTo(toDataType))
    {
      return expression;
    }

    if (!expression.DataType.IsAssignableTo(toDataType))
    {
      return null;
    }

    // Since IsAssignableTo() passed, the conversion should always succeed
    return BuildConvert(sourceLocation, scope, scopeTracker, expression, toDataType);
  }

  public ExpressionAstNode BuildAssign(
    SourceLocation sourceLocation,
    ScopeAstNode scope,
    ScopeTracker scopeTracker,
    ExpressionAstNode expression,
    AstDataType toDataType)
    => TryBuildAssign(sourceLocation, scope, scopeTracker, expression, toDataType)
    ?? throw new InvalidOperationException(
      $"Cannot assign expression of type '{expression.DataType.ToLanguageString()}' to type '{toDataType.ToLanguageString()}'");

  public ExpressionAstNode? TryBuildConvert(
    SourceLocation sourceLocation,
    ScopeAstNode scope,
    ScopeTracker scopeTracker,
    ExpressionAstNode expression,
    AstDataType toDataType)
  {
    Debug.Assert(toDataType.IsLegalValueType());
    Debug.Assert(!toDataType.IsError);

    var fromDataType = expression.DataType;
    Debug.Assert(!fromDataType.IsError);

    // Check if a conversion is even necessary
    if (fromDataType.IsIdenticalTo(toDataType))
    {
      return expression;
    }

    // We can't convert between arrays and non-arrays
    if (fromDataType.IsArray != toDataType.IsArray)
    {
      return null;
    }

    // Any empty array can be converted to an array of any type
    if (fromDataType.IsEmptyArray)
    {
      Debug.Assert(toDataType.IsArray);
      return new ChangeDataTypeAstNode(sourceLocation, expression, toDataType);
    }

    // We can use common code to handle both array and non-array conversion since array conversion simply applies to each element
    if (fromDataType.IsArray)
    {
      var singleFromDataType = fromDataType.IsArray ? fromDataType.ElementDataType() : fromDataType;
      var singleToDataType = toDataType.IsArray ? toDataType.ElementDataType() : toDataType;
      var error = false;
      var transformArray = new TransformArrayAstNode(
        sourceLocation,
        expression,
        (elementReference) =>
        {
          var transformExpression = TryBuildConvertSingle(sourceLocation, scope, scopeTracker, elementReference, singleFromDataType, singleToDataType);
          if (transformExpression == null)
          {
            error = true;
            return new PlaceholderAstNode(sourceLocation, singleToDataType);
          }

          return transformExpression;
        });

      return error ? null : transformArray;
    }
    else
    {
      return TryBuildConvertSingle(sourceLocation, scope, scopeTracker, expression, fromDataType, toDataType);
    }
  }

  public ExpressionAstNode BuildConvert(
    SourceLocation sourceLocation,
    ScopeAstNode scope,
    ScopeTracker scopeTracker,
    ExpressionAstNode expression,
    AstDataType toDataType)
    => TryBuildConvert(sourceLocation, scope, scopeTracker, expression, toDataType)
    ?? throw new InvalidOperationException(
      $"Cannot convert expression of type '{expression.DataType.ToLanguageString()}' to type '{toDataType.ToLanguageString()}'");

  private ExpressionAstNode? TryBuildConvertSingle(
    SourceLocation sourceLocation,
    ScopeAstNode scope,
    ScopeTracker scopeTracker,
    ExpressionAstNode expression,
    AstDataType fromDataType,
    AstDataType toDataType)
  {
    // In BuildConvert() we've handled a few trivial cases (types are identical, empty array conversion) and we've done array match checks so we can now just
    // rely on IsAssignableTo() to determine whether the conversion is valid (which will check things like runtime mutability and upsample factor
    // compatibility). If IsAssignableTo() returns false, we explicitly check a few more cases which are supported by conversion but not assignment.
    if (!fromDataType.IsAssignableTo(toDataType))
    {
      // If we're not directly assignable, check extra cases
      if (fromDataType.PrimitiveType != null
        && toDataType.PrimitiveType != null
        && fromDataType.RuntimeMutability >= toDataType.RuntimeMutability
        && (fromDataType.RuntimeMutability == RuntimeMutability.Constant || fromDataType.UpsampleFactor == toDataType.UpsampleFactor))
      {
        // For primitive type conversions of equal upsample factors (or from const where upsample factor doesn't matter), we'll look for a module called
        // "as <to>" which takes a single argument of type "const? <from>" (or "const from" for types which don't support variable runtime mutability). If this
        // module doesn't exist, it means the conversion is not supported.
        var moduleName = $"as {toDataType.PrimitiveType.Value.ToLanguageString()}";
        var reference = NameResolver.TryGetOrExtendReference(scope, null, sourceLocation, moduleName);
        if (reference == null)
        {
          return null;
        }

        Debug.Assert(reference is ModuleGroupReferenceAstReferenceNode);
        var moduleCallBuilder = new ModuleCallBuilder(context, defaultValueExpressionResolver);
        return moduleCallBuilder.BuildModuleCall(
          sourceLocation,
          scope,
          scopeTracker,
          reference,
          toDataType.UpsampleFactor,
          [
            new()
            {
              SourceLocation = expression.SourceLocation,
              Direction = ModuleParameterDirection.In,
              Name = null,
              ValueExpression = expression,
              SkipValueInitializedCheck = true, // We already checked this above, don't report it again
            },
          ]);
      }

      return null;
    }

    if (fromDataType.PrimitiveType != null)
    {
      Debug.Assert(fromDataType.PrimitiveType == toDataType.PrimitiveType);

      // The conversion is just a runtime mutability change
      return new ChangeDataTypeAstNode(sourceLocation, expression, toDataType);
    }

    Debug.Assert(fromDataType.StructDefinition != null);
    Debug.Assert(toDataType.StructDefinition != null);

    // We need to map fields, dropping any that aren't in toDataType. We can map fields via StructFieldAccessAstNode. The input expression should only be
    // evaluated once so we'll use a sequential evaluation node.
    var result = new SequentialEvaluationAstNode(sourceLocation);
    var expressionReference = result.AddEntry(expression);

    var fieldInitializers = new List<StructFieldInitializerAstNode>();
    foreach (var toStructField in toDataType.StructDefinition.FlattenedFields)
    {
      var toFieldDataType = toDataType.GetModifiedFieldDataType(toStructField);

      // A name match is good enough in properly written code but if the user specified duplicate fields, we should also do a type compatibility check. The
      // "from" field must be assignable to the "to" field. However, the field's data type itself may need to be modified based on the struct's modifiers so
      // we need to account for that.
      var fromStructField = fromDataType.StructDefinition.FlattenedFields.FirstOrDefault(
        (field) => field.Name == toStructField.Name && fromDataType.GetModifiedFieldDataType(field).IsAssignableTo(toFieldDataType));

      ExpressionAstNode toFieldValueExpression;
      if (fromStructField == null)
      {
        var defaultValueExpression = defaultValueExpressionResolver.ResolveStructFieldDefaultValueExpression(toStructField);

        // Because we've verified IsAssignableTo(), if we need a default value expression, it should definitely exist
        Debug.Assert(defaultValueExpression != null);
        toFieldValueExpression = defaultValueExpression;
      }
      else
      {
        var fromFieldDataType = fromDataType.GetModifiedFieldDataType(fromStructField);
        toFieldValueExpression = new StructFieldAccessAstNode(sourceLocation, expressionReference, fromStructField, fromFieldDataType);
      }

      Debug.Assert(toFieldValueExpression.DataType.IsAssignableTo(toFieldDataType));

      // The value expression is assignable but we may need to change the data type to handle minor differences like runtime mutability
      if (!toFieldDataType.IsIdenticalTo(toFieldValueExpression.DataType))
      {
        toFieldValueExpression = new ChangeDataTypeAstNode(sourceLocation, toFieldValueExpression, toFieldDataType);
      }

      fieldInitializers.Add(new StructFieldInitializerAstNode(sourceLocation, toStructField.Name, toFieldValueExpression));
    }

    result.AddEntry(new StructValueAstNode(sourceLocation, fieldInitializers, toDataType));
    return result;
  }
}