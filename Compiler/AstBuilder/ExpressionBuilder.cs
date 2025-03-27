using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Lexer;
using Compiler.Parser.Nodes;
using Compiler.Parser.Nodes.Expression;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Reflection;

namespace Compiler.AstBuilder;

file static class ReportingExtensions
{
  public static void IllegalAccessError(this IReporting reporting, AccessParseTreeNode access, ExpressionAstNode contextExpression)
    => reporting.Error("IllegalAccess", access.SourceLocation, $"Cannot resolve '{access.Identifier}' from within {contextExpression.NodeName} context");

  public static void ResolveAccessError(this IReporting reporting, AccessParseTreeNode access, AstDataType contextDataType)
    => reporting.Error("ResolveAccess", access.SourceLocation, $"Failed to resolve '{access.Identifier}' on {contextDataType.ToLanguageString()} value");

  public static void ResolveIdentifierError(this IReporting reporting, IdentifierParseTreeNode identifier)
    => reporting.Error("ResolveIdentifier", identifier.SourceLocation, $"Failed to resolve '{identifier.Identifier}'");

  public static void InvalidOperatorArgumentTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    TokenType operatorTokenType,
    int argumentIndex,
    AstDataType expectedDataType,
    AstDataType providedDataType)
  {
    var message = $"Argument {argumentIndex + 1} of {operatorTokenType.Description()} has incorrect type; "
      + $"expected '{expectedDataType.ToLanguageString()}' but got '{providedDataType.ToLanguageString()}'";
    reporting.Error("InvalidOperatorArgumentType", sourceLocation, message);
  }

  public static void NoCommonArrayDataTypeError(this IReporting reporting, ArrayParseTreeNode array)
    => reporting.Error("NoCommonArrayDataType", array.SourceLocation, "No common data type for array elements");

  public static void MultidimensionalArraysNotSupportedError(this IReporting reporting, ArrayParseTreeNode array)
    => reporting.Error("MultidimensionalArraysNotSupported", array.SourceLocation, "Multidimensional arrays are not supported");

  public static void NoCommonTernaryDataTypeError(
    this IReporting reporting,
    TernaryOperatorParseTreeNode ternaryOperator,
    AstDataType lhsDataType,
    AstDataType rhsDataType)
  {
    var message = $"No common data type for {TokenType.QuestionMark.Description()} operator arguments "
      + $"of types '{lhsDataType.ToLanguageString()}' and '{rhsDataType.ToLanguageString()}'";
    reporting.Error("NoCommonTernaryDataType", ternaryOperator.SourceLocation, message);
  }

  public static void CannotIndexNonArrayTypeError(this IReporting reporting, ArrayIndexParseTreeNode arrayIndex, AstDataType arrayDataType)
    => reporting.Error("CannotIndexNonArrayType", arrayIndex.SourceLocation, $"Cannot index non-array value of type '{arrayDataType.ToLanguageString()}'");

  public static void CannotIndexEmptyArrayError(this IReporting reporting, ArrayIndexParseTreeNode arrayIndex)
    => reporting.Error("CannotIndexEmptyArray", arrayIndex.SourceLocation, "Cannot index empty value");

  public static void CannotIndexUsingNonNumberIndexError(this IReporting reporting, ArrayIndexParseTreeNode arrayIndex, AstDataType indexDataType)
    => reporting.Error(
      "CannotIndexUsingNonNumberIndex",
      arrayIndex.SourceLocation,
      $"Cannot index array value using index of type '{indexDataType.ToLanguageString()}'");

  public static void CannotIndexUsingMismatchedUpsampleFactorError(
    this IReporting reporting,
    ArrayIndexParseTreeNode arrayIndex,
    AstDataType arrayDataType,
    AstDataType indexDataType)
  {
    var message = $"Cannot index array of type '{arrayDataType.ToLanguageString()}' using index of type '{indexDataType.ToLanguageString()}' "
      + "due to mismatched upsample factors";
    reporting.Error("CannotIndexUsingMismatchedUpsampleFactor", arrayIndex.SourceLocation, message);
  }

  public static void IllegalArrayAndIndexDataTypeCombinationError(
    this IReporting reporting,
    ArrayIndexParseTreeNode arrayIndex,
    AstDataType arrayDataType,
    AstDataType indexDataType,
    AstDataType resultingDataType)
  {
    var message = $"Cannot index array value of type '{arrayDataType.ToLanguageString()}' using index of type '{indexDataType.ToLanguageString()} "
      + $"because it would produce value of type '{resultingDataType.ToLanguageString()}' which has illegal runtime mutability";
    reporting.Error("IllegalArrayAndIndexDataTypeCombination", arrayIndex.SourceLocation, message);
  }

  public static void FieldNameConflictError(
    this IReporting reporting,
    StructFieldInitializerParseTreeNode structFieldInitializer)
    => reporting.Error(
      "FieldNameConflict",
      structFieldInitializer.SourceLocation,
      $"Name '{structFieldInitializer.Name}' conflict between multiple fields in struct value");

  public static void UnsupportedConversionError(this IReporting reporting, ConvertParseTreeNode convert, AstDataType fromDataType, AstDataType toDataType)
    => reporting.Error(
      "UnsupportedConversion",
      convert.SourceLocation,
      $"Cannot convert from type '{fromDataType.ToLanguageString()}' to type '{toDataType.ToLanguageString()}'");

  public static void IllegalTargetDataTypeError(this IReporting reporting, ConvertParseTreeNode convert)
    => reporting.Error(
      "IllegalTargetDataType",
      convert.SourceLocation,
      $"Conversion target type '{convert.DataType.ToLanguageString()}' is not a legal value type");
}

internal class BuildExpressionResult
{
  // This is the resulting built expression
  public required ExpressionAstNode Expression { get; init; }

  // If the expression resolves to true, this scope tracker should be used for the "true" branch
  public required ScopeTracker TrueBranchScopeTracker { get; init; }

  // If the expression resolves to false, this scope tracker should be used for the "false" branch
  public required ScopeTracker FalseBranchScopeTracker { get; init; }
}

// Note: evaluation order should always be left-to-right. This means if module arguments are provided by name, they should still be ordered based on how they
// were written. This also applies to assignment expressions (x = y), for loops (for (val x in y)), etc.
internal class ExpressionBuilder(AstBuilderContext context, DefaultValueExpressionResolver defaultValueExpressionResolver)
{
  // Special "field" on array types which accesses the array's length
  private const string ArrayLengthFieldName = "length";

  // Special "field" on string types which accesses the string's length
  private const string StringLengthFieldName = "length";

  // Special "field" on primitive types which accesses the primitive's latency
  private const string PrimitiveLatencyFieldName = "latency";

  // This will build the expression and modify scope/scopeTracker with any values declared/assigned within the expression (via out module arguments). The
  // resulting BuildExpressionResult will contain two child branch trackers. These should be used in the appropriate branches if the expression is used as the
  // condition in a conditional statement but can be ignored otherwise.
  public BuildExpressionResult BuildExpression(ExpressionParseTreeNode expression, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    if (expression is AccessParseTreeNode access)
    {
      return BuildAccess(access, scope, scopeTracker);
    }
    else if (expression is ArrayIndexParseTreeNode arrayIndex)
    {
      return BuildArrayIndex(arrayIndex, scope, scopeTracker);
    }
    else if (expression is ArrayParseTreeNode array)
    {
      return BuildArray(array, scope, scopeTracker);
    }
    else if (expression is ConvertParseTreeNode convert)
    {
      return BuildConvert(convert, scope, scopeTracker);
    }
    else if (expression is IdentifierParseTreeNode identifier)
    {
      return BuildIdentifier(identifier, scope, scopeTracker);
    }
    else if (expression is LiteralParseTreeNode literal)
    {
      return BuildLiteral(literal, scopeTracker);
    }
    else if (expression is ModuleCallParseTreeNode moduleCall)
    {
      return BuildModuleCall(moduleCall, scope, scopeTracker);
    }
    else if (expression is OperatorCallParseTreeNode operatorCall)
    {
      return BuildOperatorCall(operatorCall, scope, scopeTracker);
    }
    else if (expression is StructValueParseTreeNode structValue)
    {
      return BuildStructValue(structValue, scope, scopeTracker);
    }
    else if (expression is TernaryOperatorParseTreeNode ternaryOperator)
    {
      return BuildTernaryOperator(ternaryOperator, scope, scopeTracker);
    }
    else
    {
      throw new InvalidOperationException("Unknown expression type");
    }
  }

  public ExpressionAstNode BuildAssignmentOperatorCall(
    SourceLocation sourceLocation,
    TokenType operatorTokenType,
    ExpressionAstNode lhsExpression,
    ExpressionAstNode rhsExpression,
    ScopeAstNode scope,
    ScopeTracker scopeTracker)
    => BuildNonLogicalOperatorCall(sourceLocation, operatorTokenType, [lhsExpression, rhsExpression], scope, scopeTracker);

  // This should be used in cases where an expression does not use short-circuit logic but where its sub-expressions may use short-circuit logic. In this case,
  // all branches in the sub-expressions could have possibly run so they must be unioned. For example:
  //  if (Foo(a && b)) { <true> } else { <false> }
  // It is possible that "b" may not be evaluated if "a" evaluates to false, but the result of "a && b" is passed into Foo() and Foo() is always evaluated,
  // meaning that the evaluation of "b" is not tied to whether the <true> or <false> branches will run. Note that this automatically modifies the provided
  // scopeTracker by integrating each sub-result's true/false branch union (this is because each sub-result is assumed to have run).
  private static BuildExpressionResult ResultFromSubResults(
    ScopeTracker scopeTracker,
    ExpressionAstNode expression,
    IEnumerable<BuildExpressionResult> subResults)
  {
    foreach (var subResult in subResults)
    {
      var unionedBranchScopes = ScopeTracker.Union([subResult.TrueBranchScopeTracker, subResult.FalseBranchScopeTracker]);
      scopeTracker.IntegrateChildScope(unionedBranchScopes);
    }

    return new()
    {
      Expression = expression,
      TrueBranchScopeTracker = new(scopeTracker, ScopeType.Conditional),
      FalseBranchScopeTracker = new(scopeTracker, ScopeType.Conditional),
    };
  }

  private static BuildExpressionResult BuildLiteral(LiteralParseTreeNode literal, ScopeTracker scopeTracker)
  {
    var expression = literal.LiteralToken.TokenType switch
    {
      TokenType.LiteralFloat => new LiteralAstNode(literal.SourceLocation, literal.LiteralToken.LiteralFloatValue),
      TokenType.LiteralDouble => new LiteralAstNode(literal.SourceLocation, literal.LiteralToken.LiteralDoubleValue),
      TokenType.LiteralBool => new LiteralAstNode(literal.SourceLocation, literal.LiteralToken.LiteralBoolValue),
      TokenType.LiteralInt => new LiteralAstNode(literal.SourceLocation, literal.LiteralToken.LiteralIntValue),
      TokenType.LiteralString => new LiteralAstNode(literal.SourceLocation, literal.LiteralToken.LiteralStringValue),
      _ => throw UnhandledEnumValueException.Create(literal.LiteralToken.TokenType),
    };

    return ResultFromSubResults(scopeTracker, expression, []);
  }

  private BuildExpressionResult BuildAccess(AccessParseTreeNode access, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    ExpressionAstNode resultExpression;
    var contextExpressionResult = BuildExpression(access.ContextExpression, scope, scopeTracker);
    if (contextExpressionResult.Expression.DataType.IsError)
    {
      resultExpression = new PlaceholderAstNode(access.SourceLocation, AstDataType.Error());
    }
    else
    {
      context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, contextExpressionResult.Expression);

      if (contextExpressionResult.Expression.DataType.IsArray)
      {
        if (access.Identifier == ArrayLengthFieldName)
        {
          resultExpression = new ArrayLengthAstNode(access.SourceLocation, contextExpressionResult.Expression);
        }
        else
        {
          resultExpression = new PlaceholderAstNode(access.SourceLocation, AstDataType.Error());
          context.Reporting.ResolveAccessError(access, contextExpressionResult.Expression.DataType);
        }
      }
      else if (contextExpressionResult.Expression.DataType.PrimitiveType != null)
      {
        if (access.Identifier == PrimitiveLatencyFieldName)
        {
          resultExpression = new PrimitiveLatencyAstNode(access.SourceLocation, contextExpressionResult.Expression);
        }
        else if (access.Identifier == StringLengthFieldName && contextExpressionResult.Expression.DataType.PrimitiveType == PrimitiveType.String)
        {
          resultExpression = new StringLengthAstNode(access.SourceLocation, contextExpressionResult.Expression);
        }
        else
        {
          resultExpression = new PlaceholderAstNode(access.SourceLocation, AstDataType.Error());
          context.Reporting.ResolveAccessError(access, contextExpressionResult.Expression.DataType);
        }
      }
      else if (contextExpressionResult.Expression.DataType.StructDefinition != null)
      {
        var structField = contextExpressionResult.Expression.DataType.StructDefinition.FlattenedFields.FirstOrDefault(
          (field) => field.Name == access.Identifier);
        if (structField != null)
        {
          var dataType = contextExpressionResult.Expression.DataType.GetModifiedFieldDataType(structField);
          resultExpression = new StructFieldAccessAstNode(access.SourceLocation, contextExpressionResult.Expression, structField, dataType);
        }
        else
        {
          resultExpression = new PlaceholderAstNode(access.SourceLocation, AstDataType.Error());
          context.Reporting.ResolveAccessError(access, contextExpressionResult.Expression.DataType);
        }
      }
      else if (contextExpressionResult.Expression is ReferenceAstNode reference)
      {
        ExpressionAstNode? resolvedReference = NameResolver.TryGetOrExtendReference(scope, reference, access.SourceLocation, access.Identifier);
        if (resolvedReference != null)
        {
          resultExpression = resolvedReference;
        }
        else
        {
          context.Reporting.ResolveAccessError(access, reference.DataType);
          resultExpression = new PlaceholderAstNode(access.SourceLocation, AstDataType.Error());
        }
      }
      else
      {
        context.Reporting.IllegalAccessError(access, contextExpressionResult.Expression);
        resultExpression = new PlaceholderAstNode(access.SourceLocation, AstDataType.Error());
      }
    }

    return ResultFromSubResults(scopeTracker, resultExpression, [contextExpressionResult]);
  }

  private BuildExpressionResult BuildArrayIndex(ArrayIndexParseTreeNode arrayIndex, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    var arrayExpressionResult = BuildExpression(arrayIndex.ArrayExpression, scope, scopeTracker);
    var arrayExpression = arrayExpressionResult.Expression;
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, arrayExpression);

    var subResults = new List<BuildExpressionResult>() { arrayExpressionResult };
    ExpressionAstNode indexExpression;
    if (arrayIndex.IndexExpression != null)
    {
      var indexExpressionResult = BuildExpression(arrayIndex.IndexExpression, scope, scopeTracker);
      indexExpression = indexExpressionResult.Expression;
      context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, indexExpression);

      subResults.Add(indexExpressionResult);
    }
    else
    {
      // If the index expression failed to parse, just add a placeholder expression (we already emitted an error during parse)
      indexExpression = new PlaceholderAstNode(arrayIndex.SourceLocation, new(RuntimeMutability.Constant, PrimitiveType.Int, 1, false));
    }

    AstDataType elementDataType;
    if (!arrayExpression.DataType.IsArray)
    {
      elementDataType = AstDataType.Error();
      if (!arrayExpression.DataType.IsError)
      {
        context.Reporting.CannotIndexNonArrayTypeError(arrayIndex, arrayExpression.DataType);
      }
    }
    else if (arrayExpression.DataType.IsEmptyArray)
    {
      elementDataType = AstDataType.Error();
      context.Reporting.CannotIndexEmptyArrayError(arrayIndex);
    }
    else
    {
      elementDataType = arrayExpression.DataType.ElementDataType();
    }

    if (!indexExpression.DataType.IsError)
    {
      if (indexExpression.DataType.IsValidArrayIndex())
      {
        // Indexing anything using variable runtime mutability forces the resulting data type to be variable runtime mutability so we need to make sure that we
        // don't end up with an illegal element data type (e.g. mutable string).
        if (!elementDataType.IsError)
        {
          if (indexExpression.DataType.RuntimeMutability < elementDataType.RuntimeMutability)
          {
            elementDataType = elementDataType.WithRuntimeMutability(indexExpression.DataType.RuntimeMutability);
            if (!elementDataType.IsLegal())
            {
              context.Reporting.IllegalArrayAndIndexDataTypeCombinationError(arrayIndex, arrayExpression.DataType, indexExpression.DataType, elementDataType);
              elementDataType = AstDataType.Error();
            }
          }
        }
      }
      else
      {
        context.Reporting.CannotIndexUsingNonNumberIndexError(arrayIndex, indexExpression.DataType);
      }

      if (!arrayExpression.DataType.IsError
        && arrayExpression.DataType.RuntimeMutability != RuntimeMutability.Constant
        && indexExpression.DataType.RuntimeMutability != RuntimeMutability.Constant
        && arrayExpression.DataType.UpsampleFactor != indexExpression.DataType.UpsampleFactor)
      {
        context.Reporting.CannotIndexUsingMismatchedUpsampleFactorError(arrayIndex, arrayExpression.DataType, indexExpression.DataType);
      }
    }

    return ResultFromSubResults(
      scopeTracker,
      new ArrayIndexAstNode(arrayIndex.SourceLocation, arrayExpression, indexExpression, elementDataType),
      subResults);
  }

  private BuildExpressionResult BuildArray(ArrayParseTreeNode array, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    var elementResults = array.Elements
      .Select(
        (element) =>
        {
          var elementExpression = BuildExpression(element, scope, scopeTracker);
          context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, elementExpression.Expression);
          return elementExpression;
        })
      .ToArray();

    AstDataType dataType;
    if (elementResults.IsEmpty())
    {
      dataType = AstDataType.EmptyArray();
    }
    else
    {
      var nonErrorElementDataTypes = elementResults.Select((element) => element.Expression.DataType).Where((dataType) => !dataType.IsError).ToArray();
      if (nonErrorElementDataTypes.IsEmpty())
      {
        dataType = AstDataType.Error();
      }
      else
      {
        var elementDataType = nonErrorElementDataTypes.CommonDataType();
        if (elementDataType.IsError)
        {
          context.Reporting.NoCommonArrayDataTypeError(array);
          dataType = AstDataType.Error();
        }
        else if (elementDataType.IsArray)
        {
          context.Reporting.MultidimensionalArraysNotSupportedError(array);
          dataType = AstDataType.Error();
        }
        else
        {
          dataType = elementDataType.ArrayDataType();
        }
      }
    }

    return ResultFromSubResults(
      scopeTracker,
      new ArrayAstNode(array.SourceLocation, elementResults.Select((element) => element.Expression).ToArray(), dataType),
      elementResults);
  }

  private BuildExpressionResult BuildConvert(ConvertParseTreeNode convert, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    var expressionResult = BuildExpression(convert.Expression, scope, scopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, expressionResult.Expression);

    var fromDataType = expressionResult.Expression.DataType;
    var toDataType = convert.DataType.ToAstDataType(scope, context.Reporting);
    if (!toDataType.IsLegalValueType())
    {
      context.Reporting.IllegalTargetDataTypeError(convert);
      toDataType = AstDataType.Error();
    }

    if (toDataType.RuntimeMutability == RuntimeMutability.DependentConstant && !(scope.ModuleDefinition?.IsDependentConstant ?? false))
    {
      context.Reporting.IllegalDependentConstantDataTypeError(convert.DataType.SourceLocation, toDataType);
    }

    if (fromDataType.IsError || toDataType.IsError)
    {
      return ResultFromSubResults(
        scopeTracker,
        new PlaceholderAstNode(convert.SourceLocation, toDataType),
        [expressionResult]);
    }

    var convertBuilder = new ConvertBuilder(context, defaultValueExpressionResolver);
    var result = convertBuilder.TryBuildConvert(convert.SourceLocation, scope, scopeTracker, expressionResult.Expression, toDataType);
    if (result == null)
    {
      context.Reporting.UnsupportedConversionError(convert, fromDataType, toDataType);
      result = new PlaceholderAstNode(convert.SourceLocation, toDataType);
    }

    return ResultFromSubResults(scopeTracker, result, [expressionResult]);
  }

  private BuildExpressionResult BuildIdentifier(IdentifierParseTreeNode identifier, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    ExpressionAstNode? reference = NameResolver.TryGetOrExtendReference(scope, null, identifier.SourceLocation, identifier.Identifier);
    if (reference == null)
    {
      context.Reporting.ResolveIdentifierError(identifier);
      reference = new PlaceholderAstNode(identifier.SourceLocation, AstDataType.Error());
    }

    return ResultFromSubResults(scopeTracker, reference, []);
  }

  private BuildExpressionResult BuildModuleCall(ModuleCallParseTreeNode moduleCall, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    var moduleExpressionResult = BuildExpression(moduleCall.ModuleNameExpression, scope, scopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, moduleExpressionResult.Expression);

    // Don't check initialization state here, ModuleCallBuilder will do that. It's fine for output arguments to be uninitialized.
    var argumentExpressionResults = moduleCall.Arguments.Select((argument) => BuildExpression(argument.ValueExpression, scope, scopeTracker)).ToArray();

    var moduleCallBuilder = new ModuleCallBuilder(context, defaultValueExpressionResolver);
    var expression = moduleCallBuilder.BuildModuleCall(
      moduleCall.SourceLocation,
      scope,
      scopeTracker,
      moduleExpressionResult.Expression,
      moduleCall.UpsampleFactor,
      moduleCall.Arguments
        .Zip(argumentExpressionResults)
        .Select(
          (entry) => new ModuleCallBuilder.Argument()
          {
            SourceLocation = entry.Second.Expression.SourceLocation,
            Direction = entry.First.Direction,
            Name = entry.First.Name,
            ValueExpression = entry.Second.Expression,
          })
        .ToArray());

    return ResultFromSubResults(scopeTracker, expression, [moduleExpressionResult, ..argumentExpressionResults]);
  }

  private BuildExpressionResult BuildOperatorCall(OperatorCallParseTreeNode operatorCall, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    // Handle logical operators as a special case because they are short-circuiting
    if (operatorCall.OperatorTokenType == TokenType.LogicalOr)
    {
      return BuildLogicalOrOperatorCall(operatorCall, scope, scopeTracker);
    }
    else if (operatorCall.OperatorTokenType == TokenType.LogicalAnd)
    {
      return BuildLogicalAndOperatorCall(operatorCall, scope, scopeTracker);
    }
    else if (operatorCall.OperatorTokenType == TokenType.LogicalNot)
    {
      return BuildLogicalNotOperatorCall(operatorCall, scope, scopeTracker);
    }
    else
    {
      // Don't check argument initialization state here, that will happen within BuildNonLogicalOperatorCall()
      var argumentExpressionResults = operatorCall.Arguments.Select((argument) => BuildExpression(argument.ValueExpression, scope, scopeTracker)).ToArray();

      var result = BuildNonLogicalOperatorCall(
          operatorCall.SourceLocation,
          operatorCall.OperatorTokenType,
          argumentExpressionResults.Select((v) => v.Expression).ToArray(),
          scope,
          scopeTracker);
      return ResultFromSubResults(scopeTracker, result, argumentExpressionResults);
    }
  }

  private ExpressionAstNode BuildNonLogicalOperatorCall(
    SourceLocation sourceLocation,
    TokenType operatorTokenType,
    IReadOnlyList<ExpressionAstNode> argumentExpressions,
    ScopeAstNode scope,
    ScopeTracker scopeTracker)
  {
    // Handle array + and * operators as a special case because they work on all element types and we don't support templates/generics
    var arrayOperationResult = TryBuildArrayOperation(sourceLocation, operatorTokenType, scope, scopeTracker, argumentExpressions);
    if (arrayOperationResult != null)
    {
      return arrayOperationResult;
    }

    // Call the module associated with the operator. Its name is simply the operator's symbol text.
    var operatorField = typeof(TokenType).GetField(operatorTokenType.ToString());
    Debug.Assert(operatorField != null);

    var moduleName = operatorField.GetCustomAttribute<SymbolTokenAttribute>()?.Text;
    Debug.Assert(moduleName != null);

    var reference = NameResolver.TryGetOrExtendReference(scope, null, sourceLocation, moduleName);
    Debug.Assert(reference != null);

    Debug.Assert(reference is ModuleGroupReferenceAstReferenceNode);
    var moduleCallBuilder = new ModuleCallBuilder(context, defaultValueExpressionResolver);
    return moduleCallBuilder.BuildModuleCall(
      sourceLocation,
      scope,
      scopeTracker,
      reference,
      null,
      argumentExpressions
        .Select(
          (argumentExpression) => new ModuleCallBuilder.Argument()
          {
            SourceLocation = argumentExpression.SourceLocation,
            Direction = ModuleParameterDirection.In,
            Name = null,
            ValueExpression = argumentExpression,
          })
        .ToArray());
  }

  private BuildExpressionResult BuildLogicalOrOperatorCall(OperatorCallParseTreeNode operatorCall, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    Debug.Assert(operatorCall.OperatorTokenType == TokenType.LogicalOr);

    // The block of code:
    //
    //  lhs || rhs
    //
    // is logically equivalent to:
    //
    //  if (lhs)
    //  {
    //    return true;
    //  }
    //  else
    //  {
    //    return rhs;
    //  }

    // This will evaluate the first argument and modify commit
    var lhsResult = BuildExpression(operatorCall.Arguments[0].ValueExpression, scope, scopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, lhsResult.Expression);

    // If the LHS is true, we enter the "return true" scope above
    var lhsTrueScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);

    // If the LHS is false, we enter the "return rhs" scope above, where the RHS is evaluated
    var lhsFalseScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    var rhsResult = BuildExpression(operatorCall.Arguments[1].ValueExpression, scope, lhsFalseScopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, rhsResult.Expression);

    // We've run both LHS branches so we need to update the outer scope by integrating both of them
    scopeTracker.IntegrateChildScope(ScopeTracker.Union([lhsTrueScopeTracker, lhsFalseScopeTracker]));

    // We need to bring the LHS false branches down a level for correctness
    var lhsFalseRhsTrueScopeTracker = lhsFalseScopeTracker.Clone();
    lhsFalseRhsTrueScopeTracker.IntegrateChildScope(rhsResult.TrueBranchScopeTracker);
    var lhsFalseRhsFalseScopeTracker = lhsFalseScopeTracker.Clone();
    lhsFalseRhsFalseScopeTracker.IntegrateChildScope(rhsResult.FalseBranchScopeTracker);

    // Create "true" and "false" scopes to use for branches if this expression is part of a conditional. These scopes are children of our outer scope.

    // The "true" branch runs when either an LHS true branch runs OR when an LHS false branch runs and an RHS true branch runs
    var trueBranchScopeTrackerA = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    trueBranchScopeTrackerA.IntegrateChildScope(lhsResult.TrueBranchScopeTracker);
    var trueBranchScopeTrackerB = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    trueBranchScopeTrackerB.IntegrateChildScope(lhsResult.FalseBranchScopeTracker);
    trueBranchScopeTrackerB.IntegrateChildScope(lhsFalseRhsTrueScopeTracker);
    var trueBranchScopeTracker = ScopeTracker.Union([trueBranchScopeTrackerA, trueBranchScopeTrackerB]);

    // The "false" branch runs when an LHS false branch runs and an RHS false branch runs
    var falseBranchScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    falseBranchScopeTracker.IntegrateChildScope(lhsResult.FalseBranchScopeTracker);
    falseBranchScopeTracker.IntegrateChildScope(lhsFalseRhsFalseScopeTracker);

    var constBoolDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false);

    if (!lhsResult.Expression.DataType.IsAssignableTo(constBoolDataType))
    {
      context.Reporting.InvalidOperatorArgumentTypeError(
        operatorCall.SourceLocation,
        operatorCall.OperatorTokenType,
        0,
        constBoolDataType,
        lhsResult.Expression.DataType);
    }

    if (!rhsResult.Expression.DataType.IsAssignableTo(constBoolDataType))
    {
      context.Reporting.InvalidOperatorArgumentTypeError(
        operatorCall.SourceLocation,
        operatorCall.OperatorTokenType,
        1,
        constBoolDataType,
        rhsResult.Expression.DataType);
    }

    return new()
    {
      Expression = new TernaryAstNode(
        operatorCall.SourceLocation,
        lhsResult.Expression,
        new LiteralAstNode(operatorCall.SourceLocation, true),
        rhsResult.Expression,
        constBoolDataType),
      TrueBranchScopeTracker = trueBranchScopeTracker,
      FalseBranchScopeTracker = falseBranchScopeTracker,
    };
  }

  private BuildExpressionResult BuildLogicalAndOperatorCall(OperatorCallParseTreeNode operatorCall, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    Debug.Assert(operatorCall.OperatorTokenType == TokenType.LogicalAnd);

    // The block of code:
    //
    //  lhs && rhs
    //
    // is logically equivalent to:
    //
    //  if (lhs)
    //  {
    //    return rhs;
    //  }
    //  else
    //  {
    //    return false;
    //  }

    // This will evaluate the first argument and modify scopeTracker
    var lhsResult = BuildExpression(operatorCall.Arguments[0].ValueExpression, scope, scopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, lhsResult.Expression);

    // If the LHS is true, we enter the "return rhs" scope above, where the RHS is evaluated
    var lhsTrueScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    var rhsResult = BuildExpression(operatorCall.Arguments[1].ValueExpression, scope, lhsTrueScopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, rhsResult.Expression);

    // If the LHS is false, we enter the "return false" scope above
    var lhsFalseScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);

    // We've run both LHS branches so we need to update the outer scope by integrating both of them
    scopeTracker.IntegrateChildScope(ScopeTracker.Union([lhsTrueScopeTracker, lhsFalseScopeTracker]));

    // We need to bring the LHS false branches down a level for correctness
    var lhsTrueRhsTrueScopeTracker = lhsTrueScopeTracker.Clone();
    lhsTrueRhsTrueScopeTracker.IntegrateChildScope(rhsResult.TrueBranchScopeTracker);
    var lhsTrueRhsFalseScopeTracker = lhsTrueScopeTracker.Clone();
    lhsTrueRhsFalseScopeTracker.IntegrateChildScope(rhsResult.FalseBranchScopeTracker);

    // Create "true" and "false" scopes to use for branches if this expression is part of a conditional. These scopes are children of our outer scope.

    // The "true" branch runs when an LHS true branch runs and an RHS true branch runs
    var trueBranchScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    trueBranchScopeTracker.IntegrateChildScope(lhsResult.TrueBranchScopeTracker);
    trueBranchScopeTracker.IntegrateChildScope(lhsTrueRhsTrueScopeTracker);

    // The "false" branch runs when either an LHS false branch runs OR when an LHS true branch runs and an RHS false branch runs
    var falseBranchScopeTrackerA = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    falseBranchScopeTrackerA.IntegrateChildScope(lhsResult.FalseBranchScopeTracker);
    var falseBranchScopeTrackerB = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    falseBranchScopeTrackerB.IntegrateChildScope(lhsResult.TrueBranchScopeTracker);
    falseBranchScopeTrackerB.IntegrateChildScope(lhsTrueRhsFalseScopeTracker);
    var falseBranchScopeTracker = ScopeTracker.Union([falseBranchScopeTrackerA, falseBranchScopeTrackerB]);

    var constBoolDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false);

    if (!lhsResult.Expression.DataType.IsAssignableTo(constBoolDataType))
    {
      context.Reporting.InvalidOperatorArgumentTypeError(
        operatorCall.SourceLocation,
        operatorCall.OperatorTokenType,
        0,
        constBoolDataType,
        lhsResult.Expression.DataType);
    }

    if (!rhsResult.Expression.DataType.IsAssignableTo(constBoolDataType))
    {
      context.Reporting.InvalidOperatorArgumentTypeError(
        operatorCall.SourceLocation,
        operatorCall.OperatorTokenType,
        1,
        constBoolDataType,
        rhsResult.Expression.DataType);
    }

    return new()
    {
      Expression = new TernaryAstNode(
        operatorCall.SourceLocation,
        lhsResult.Expression,
        rhsResult.Expression,
        new LiteralAstNode(operatorCall.SourceLocation, false),
        constBoolDataType),
      TrueBranchScopeTracker = trueBranchScopeTracker,
      FalseBranchScopeTracker = falseBranchScopeTracker,
    };
  }

  private BuildExpressionResult BuildLogicalNotOperatorCall(OperatorCallParseTreeNode operatorCall, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    Debug.Assert(operatorCall.OperatorTokenType == TokenType.LogicalNot);

    // The block of code:
    //
    //  !condition
    //
    // is logically equivalent to:
    //
    //  if (condition)
    //  {
    //    return false;
    //  }
    //  else
    //  {
    //    return true;
    //  }

    var result = BuildExpression(operatorCall.Arguments[0].ValueExpression, scope, scopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, result.Expression);

    // Create "true" and "false" scopes to use for branches if this expression is part of a conditional. These scopes are children of our outer scope.

    // The "true" branch runs when the condition is false
    var trueBranchScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    trueBranchScopeTracker.IntegrateChildScope(result.FalseBranchScopeTracker);

    // The "false" branch runs when the condition is true
    var falseBranchScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    falseBranchScopeTracker.IntegrateChildScope(result.TrueBranchScopeTracker);

    var constBoolDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false);

    if (!result.Expression.DataType.IsAssignableTo(constBoolDataType))
    {
      context.Reporting.InvalidOperatorArgumentTypeError(
        operatorCall.SourceLocation,
        operatorCall.OperatorTokenType,
        0,
        constBoolDataType,
        result.Expression.DataType);
    }

    return new()
    {
      Expression = new TernaryAstNode(
        operatorCall.SourceLocation,
        result.Expression,
        new LiteralAstNode(operatorCall.SourceLocation, false),
        new LiteralAstNode(operatorCall.SourceLocation, true),
        constBoolDataType),
      TrueBranchScopeTracker = trueBranchScopeTracker,
      FalseBranchScopeTracker = falseBranchScopeTracker,
    };
  }

  private SequentialEvaluationAstNode? TryBuildArrayOperation(
    SourceLocation sourceLocation,
    TokenType operatorTokenType,
    ScopeAstNode scope,
    ScopeTracker scopeTracker,
    IReadOnlyList<ExpressionAstNode> argumentExpressions)
  {
    if (operatorTokenType == TokenType.Plus)
    {
      if (argumentExpressions.Count != 2 || !argumentExpressions.All((v) => v.DataType.IsArray))
      {
        return null;
      }

      var commonDataType = argumentExpressions.Select((v) => v.DataType).CommonDataType();
      if (commonDataType.IsError)
      {
        return null;
      }

      foreach (var argumentExpression in argumentExpressions)
      {
        context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, argumentExpression);
      }

      // Create a sequential evaluation node because we may need to convert one of the arguments
      var result = new SequentialEvaluationAstNode(sourceLocation);

      var convertBuilder = new ConvertBuilder(context, defaultValueExpressionResolver);
      var convertedArgumentExpressions = argumentExpressions
        .Select((argumentExpression) => result.AddEntry(convertBuilder.BuildAssign(sourceLocation, scope, scopeTracker, argumentExpression, commonDataType)))
        .ToArray();

      result.AddEntry(new ArrayConcatenateAstNode(sourceLocation, convertedArgumentExpressions[0], convertedArgumentExpressions[1], commonDataType));
      return result;
    }
    else if (operatorTokenType == TokenType.Multiply)
    {
      if (argumentExpressions.Count != 2)
      {
        return null;
      }

      var arrayExpression = argumentExpressions.FirstOrDefault((expression) => expression.DataType.IsArray);
      var countExpression = argumentExpressions.FirstOrDefault(
        (expression) => expression.DataType.RuntimeMutability == RuntimeMutability.Constant
          && expression.DataType.PrimitiveType == PrimitiveType.Int
          && !expression.DataType.IsArray);

      if (arrayExpression == null || countExpression == null)
      {
        return null;
      }

      Debug.Assert(arrayExpression != countExpression);

      foreach (var argumentExpression in argumentExpressions)
      {
        context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, argumentExpression);
      }

      // Create a sequential evaluation node to make sure the array and count arguments are evaluated in left-to-right order
      var result = new SequentialEvaluationAstNode(sourceLocation);
      foreach (var argumentExpression in argumentExpressions)
      {
        result.AddEntry(argumentExpression);
      }

      result.AddEntry(new ArrayRepeatAstNode(sourceLocation, arrayExpression, countExpression, arrayExpression.DataType));
      return result;
    }

    return null;
  }

  private BuildExpressionResult BuildStructValue(StructValueParseTreeNode structValue, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    var structDefinition = new UnnamedStructDefinitionAstNode(structValue.SourceLocation);
    structDefinition.InitializeFields();

    var subResults = new List<BuildExpressionResult>();
    var fieldInitializers = new List<StructFieldInitializerAstNode>();

    foreach (var fieldInitializer in structValue.FieldInitializers)
    {
      var valueExpressionResult = BuildExpression(fieldInitializer.ValueExpression, scope, scopeTracker);
      context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, valueExpressionResult.Expression);
      subResults.Add(valueExpressionResult);

      if (fieldInitializers.Any((existingFieldInitializer) => fieldInitializer.Name == existingFieldInitializer.Name))
      {
        context.Reporting.FieldNameConflictError(fieldInitializer);
      }

      var structField = new StructFieldAstNode(fieldInitializer.SourceLocation, fieldInitializer.Name, valueExpressionResult.Expression.DataType, false);
      structField.InitializeDefaultValueExpression(null);
      structDefinition.AddField(structField);
      structDefinition.AddFlattenedField(structField);

      fieldInitializers.Add(new StructFieldInitializerAstNode(fieldInitializer.SourceLocation, fieldInitializer.Name, valueExpressionResult.Expression));
    }

    return ResultFromSubResults(
      scopeTracker,
      new StructValueAstNode(structValue.SourceLocation, fieldInitializers, new AstDataType(RuntimeMutability.Variable, structDefinition, 1, false)),
      subResults);
  }

  private BuildExpressionResult BuildTernaryOperator(TernaryOperatorParseTreeNode ternaryOperator, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    // The block of code:
    //
    //  cond ? t : f
    //
    // is logically equivalent to:
    //
    //  if (cond)
    //  {
    //    return t;
    //  }
    //  else
    //  {
    //    return f;
    //  }

    // This will evaluate the first argument and modify scopeTracker
    var conditionResult = BuildExpression(ternaryOperator.ConditionExpression, scope, scopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, conditionResult.Expression);

    // If the condition is true, we enter the "return t" scope above, where the true expression is evaluated
    var trueScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    var trueResult = BuildExpression(ternaryOperator.Arguments[0].ValueExpression, scope, trueScopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, trueResult.Expression);

    // If the condition is false, we enter the "return f" scope above, where the false expression is evaluated
    var falseScopeTracker = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    var falseResult = BuildExpression(ternaryOperator.Arguments[1].ValueExpression, scope, falseScopeTracker);
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, falseResult.Expression);

    // We've run both true and false branches so we need to update the outer scope by integrating both of them
    scopeTracker.IntegrateChildScope(ScopeTracker.Union([trueScopeTracker, falseScopeTracker]));

    // We need to bring the true/false branches down a level for correctness
    var trueTrueScopeTracker = trueScopeTracker.Clone();
    trueTrueScopeTracker.IntegrateChildScope(trueResult.TrueBranchScopeTracker);
    var trueFalseScopeTracker = trueScopeTracker.Clone();
    trueFalseScopeTracker.IntegrateChildScope(trueResult.FalseBranchScopeTracker);
    var falseTrueScopeTracker = falseScopeTracker.Clone();
    falseTrueScopeTracker.IntegrateChildScope(falseResult.TrueBranchScopeTracker);
    var falseFalseScopeTracker = falseScopeTracker.Clone();
    falseFalseScopeTracker.IntegrateChildScope(falseResult.FalseBranchScopeTracker);

    // Create "true" and "false" scopes to use for branches if this expression is part of a conditional. These scopes are children of our outer scope.

    // The "true" branch runs when a condition true branch runs and a true true branch runs or when a condition false branch runs and a false true branch runs
    var trueBranchScopeTrackerA = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    trueBranchScopeTrackerA.IntegrateChildScope(conditionResult.TrueBranchScopeTracker);
    trueBranchScopeTrackerA.IntegrateChildScope(trueTrueScopeTracker);
    var trueBranchScopeTrackerB = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    trueBranchScopeTrackerB.IntegrateChildScope(conditionResult.FalseBranchScopeTracker);
    trueBranchScopeTrackerB.IntegrateChildScope(falseTrueScopeTracker);
    var trueBranchScopeTracker = ScopeTracker.Union([trueBranchScopeTrackerA, trueBranchScopeTrackerB]);

    // The "false" branch runs when a condition true branch runs and a true false branch runs or when a condition false branch runs and a false false branch runs
    var falseBranchScopeTrackerA = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    falseBranchScopeTrackerA.IntegrateChildScope(conditionResult.TrueBranchScopeTracker);
    falseBranchScopeTrackerA.IntegrateChildScope(trueFalseScopeTracker);
    var falseBranchScopeTrackerB = new ScopeTracker(scopeTracker, ScopeType.Conditional);
    falseBranchScopeTrackerB.IntegrateChildScope(conditionResult.FalseBranchScopeTracker);
    falseBranchScopeTrackerB.IntegrateChildScope(falseFalseScopeTracker);
    var falseBranchScopeTracker = ScopeTracker.Union([falseBranchScopeTrackerA, falseBranchScopeTrackerB]);

    var constBoolDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false);

    if (!conditionResult.Expression.DataType.IsAssignableTo(constBoolDataType))
    {
      context.Reporting.InvalidOperatorArgumentTypeError(
        ternaryOperator.SourceLocation,
        TokenType.QuestionMark,
        0,
        constBoolDataType,
        conditionResult.Expression.DataType);
    }

    var commonDataType = new[] { trueResult.Expression.DataType, falseResult.Expression.DataType }.CommonDataType();
    if (commonDataType.IsError && !trueResult.Expression.DataType.IsError && !falseResult.Expression.DataType.IsError)
    {
      context.Reporting.NoCommonTernaryDataTypeError(ternaryOperator, trueResult.Expression.DataType, falseResult.Expression.DataType);
    }

    return new()
    {
      Expression = new TernaryAstNode(
        ternaryOperator.SourceLocation,
        conditionResult.Expression,
        trueResult.Expression,
        falseResult.Expression,
        commonDataType),
      TrueBranchScopeTracker = trueBranchScopeTracker,
      FalseBranchScopeTracker = falseBranchScopeTracker,
    };
  }
}