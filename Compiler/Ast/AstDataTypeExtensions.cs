using Compiler.Types;
using System.Diagnostics;

namespace Compiler.Ast;

internal static class AstDataTypeExtensions
{
  public static AstDataType CommonDataType(this IEnumerable<AstDataType> dataTypes)
  {
    AstDataType? commonDataType = null;

    foreach (var dataType in dataTypes)
    {
      // Ignore error data types in favor of non-error ones. If all data types are errors, we'll end up with error at the end.
      if (dataType.IsError)
      {
        continue;
      }

      // Use the base-most data type (if there is one)
      if (commonDataType == null)
      {
        commonDataType = dataType;
      }
      else if (commonDataType.IsAssignableTo(dataType))
      {
        commonDataType = dataType;
      }
      else if (!dataType.IsAssignableTo(commonDataType))
      {
        return AstDataType.Error();
      }
    }

    return commonDataType ?? AstDataType.Error();
  }

  public static AstDataType WithRuntimeMutability(this AstDataType dataType, RuntimeMutability runtimeMutability)
  {
    if (dataType.IsModule || dataType.IsScope || dataType.IsEmptyArray || dataType.IsError)
    {
      // These data types always have fixed runtime mutability
      return dataType;
    }

    var upsampleFactor = runtimeMutability == RuntimeMutability.Constant ? 1 : dataType.UpsampleFactor;
    if (dataType.PrimitiveType != null)
    {
      return new(runtimeMutability, dataType.PrimitiveType.Value, upsampleFactor, dataType.IsArray);
    }
    else
    {
      Debug.Assert(dataType.StructDefinition != null);
      return new(runtimeMutability, dataType.StructDefinition, upsampleFactor, dataType.IsArray);
    }
  }

  public static AstDataType WithUpsampleFactor(this AstDataType dataType, int upsampleFactor)
  {
    if (dataType.IsEmptyArray || dataType.IsModule || dataType.IsScope || dataType.IsVoid || dataType.IsError)
    {
      // These data types always have fixed upsample factor
      return dataType;
    }

    var fixedUpUpsampleFactor = dataType.RuntimeMutability == RuntimeMutability.Constant ? 1 : upsampleFactor;
    if (dataType.PrimitiveType != null)
    {
      return new(dataType.RuntimeMutability, dataType.PrimitiveType.Value, fixedUpUpsampleFactor, dataType.IsArray);
    }
    else
    {
      Debug.Assert(dataType.StructDefinition != null);
      return new(dataType.RuntimeMutability, dataType.StructDefinition, fixedUpUpsampleFactor, dataType.IsArray);
    }
  }

  public static AstDataType ElementDataType(this AstDataType arrayDataType)
  {
    if (arrayDataType.IsError)
    {
      return arrayDataType;
    }

    if (!arrayDataType.IsArray)
    {
      throw new InvalidOperationException("Cannot get element data type from non-array data type");
    }

    if (arrayDataType.IsEmptyArray)
    {
      throw new InvalidOperationException("Cannot get element data type from empty array data type");
    }

    if (arrayDataType.PrimitiveType != null)
    {
      return new(arrayDataType.RuntimeMutability, arrayDataType.PrimitiveType.Value, arrayDataType.UpsampleFactor, false);
    }
    else
    {
      Debug.Assert(arrayDataType.StructDefinition != null);
      return new(arrayDataType.RuntimeMutability, arrayDataType.StructDefinition, arrayDataType.UpsampleFactor, false);
    }
  }

  public static AstDataType ArrayDataType(this AstDataType elementDataType)
  {
    if (elementDataType.IsError)
    {
      return elementDataType;
    }

    if (elementDataType.IsArray)
    {
      throw new InvalidOperationException("Cannot get array data type from array data type");
    }

    if (elementDataType.PrimitiveType != null)
    {
      return new(elementDataType.RuntimeMutability, elementDataType.PrimitiveType.Value, elementDataType.UpsampleFactor, true);
    }
    else
    {
      Debug.Assert(elementDataType.StructDefinition != null);
      return new(elementDataType.RuntimeMutability, elementDataType.StructDefinition, elementDataType.UpsampleFactor, true);
    }
  }

  public static bool IsIdenticalTo(this AstDataType dataType, AstDataType otherDataType)
  {
    if (dataType.IsError || otherDataType.IsError)
    {
      return false;
    }

    if (dataType.IsEmptyArray || otherDataType.IsEmptyArray)
    {
      // If both are empty array, they are identical, but if only one is, they are not
      return dataType.IsEmptyArray && otherDataType.IsEmptyArray;
    }

    if (dataType.IsVoid || otherDataType.IsVoid)
    {
      // If both are void, they are identical, but if only one is, they are not
      return dataType.IsVoid && otherDataType.IsVoid;
    }

    if (dataType.IsModule || otherDataType.IsModule || dataType.IsScope || otherDataType.IsScope)
    {
      // Module and scope equivalence is not supported
      return false;
    }

    if (dataType.RuntimeMutability != otherDataType.RuntimeMutability
      || dataType.PrimitiveType != otherDataType.PrimitiveType
      || (dataType.StructDefinition == null) != (otherDataType.StructDefinition == null)
      || dataType.UpsampleFactor != otherDataType.UpsampleFactor
      || dataType.IsArray != otherDataType.IsArray)
    {
      return false;
    }

    if (dataType.StructDefinition != null)
    {
      Debug.Assert(otherDataType.StructDefinition != null);
      if (dataType.StructDefinition is NamedStructDefinitionAstNode && otherDataType.StructDefinition is NamedStructDefinitionAstNode)
      {
        if (dataType.StructDefinition != otherDataType.StructDefinition)
        {
          return false;
        }
      }
      else if (dataType.StructDefinition is UnnamedStructDefinitionAstNode && otherDataType.StructDefinition is UnnamedStructDefinitionAstNode)
      {
        // Unnamed structs with identical fields are considered identical
        var sortedFields = dataType.StructDefinition.FlattenedFields.OrderBy((field) => field.Name).ToArray();
        var otherSortedFields = otherDataType.StructDefinition.FlattenedFields.OrderBy((field) => field.Name).ToArray();
        if (sortedFields.Length != otherSortedFields.Length)
        {
          return false;
        }

        var allFieldsIdentical = sortedFields
          .Zip(otherSortedFields)
          .All((v) => v.First.Name == v.Second.Name && v.First.DataType.IsIdenticalTo(v.Second.DataType));
        if (!allFieldsIdentical)
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }

    return true;
  }

  public static bool IsAssignableTo(this AstDataType fromDataType, AstDataType toDataType)
  {
    // Error types are considered compatible with any other type. This is because any time an error type arises, we've already reported an error, so we don't
    // want to trigger a cascade of a bunch of additional downstream errors.
    if (fromDataType.IsError || toDataType.IsError)
    {
      return true;
    }

    // Module types only exist for identifying module references and don't support assignment
    if (fromDataType.IsModule || toDataType.IsModule)
    {
      return false;
    }

    // It doesn't make sense to assign T[] to T or T to T[]
    if (fromDataType.IsArray != toDataType.IsArray)
    {
      return false;
    }

    // Empty arrays can be assigned to any array target type
    if (fromDataType.IsEmptyArray)
    {
      return toDataType.IsArray;
    }

    // If the target type is an empty array, it's only safe to assign another empty array
    if (toDataType.IsEmptyArray)
    {
      return fromDataType.IsEmptyArray;
    }

    if (fromDataType.PrimitiveType != null)
    {
      // The primitive type must match (we don't allow implicit float -> double or double -> float conversions)
      if (fromDataType.PrimitiveType != toDataType.PrimitiveType)
      {
        return false;
      }
    }
    else if (fromDataType.IsVoid)
    {
      if (!toDataType.IsVoid)
      {
        return false;
      }
    }
    else
    {
      Debug.Assert(fromDataType.StructDefinition != null);

      // Unnamed structs require special logic
      if (fromDataType.StructDefinition is UnnamedStructDefinitionAstNode)
      {
        return IsUnnamedStructAssignableTo(fromDataType, toDataType);
      }

      // The target type must be a struct but it doesn't need be an exact match
      if (toDataType.StructDefinition == null)
      {
        return false;
      }

      // Named struct values can be assigned to the same struct type or to a base type. They cannot be assigned to unnamed struct types.
      if (!toDataType.StructDefinition.IsBaseOf(fromDataType.StructDefinition))
      {
        return false;
      }
    }

    // You can't assign T to const T but you can assign const T to T (or const T to const? T or const? T to T)
    if (fromDataType.RuntimeMutability < toDataType.RuntimeMutability)
    {
      return false;
    }

    // The upsample factor of const values is always 1 so if we're going from const T to T@2x, for example, the upsample factor doesn't need to match
    if (fromDataType.RuntimeMutability != RuntimeMutability.Constant && fromDataType.UpsampleFactor != toDataType.UpsampleFactor)
    {
      return false;
    }

    return true;
  }

  // If the struct is marked as const, all fields get upgraded to const, and the struct's upsample factor gets multiplied across all fields
  public static AstDataType GetModifiedFieldDataType(this AstDataType structDataType, StructFieldAstNode structField)
  {
    Debug.Assert(structDataType.StructDefinition != null);
    Debug.Assert(structDataType.StructDefinition.FlattenedFields.Contains(structField));
    return structField.DataType
      .WithRuntimeMutability((RuntimeMutability)Math.Max((int)structDataType.RuntimeMutability, (int)structField.DataType.RuntimeMutability))
      .WithUpsampleFactor(structDataType.UpsampleFactor * structField.DataType.UpsampleFactor);
  }

  public static bool IsNumber(this AstDataType dataType)
    => !dataType.IsArray
    && (dataType.PrimitiveType == PrimitiveType.Float || dataType.PrimitiveType == PrimitiveType.Double || dataType.PrimitiveType == PrimitiveType.Int);

  public static bool IsValidArrayIndex(this AstDataType dataType)
    => dataType.IsNumber();

  private static bool IsUnnamedStructAssignableTo(AstDataType fromDataType, AstDataType toDataType)
  {
    Debug.Assert(fromDataType.StructDefinition is UnnamedStructDefinitionAstNode);
    Debug.Assert(fromDataType.IsArray == toDataType.IsArray);

    // Unnamed structs themselves should never be const or upsampled, these properties can be on the fields
    Debug.Assert(fromDataType.RuntimeMutability == RuntimeMutability.Variable);
    Debug.Assert(fromDataType.UpsampleFactor == 1);

    // The target type must be a struct but it doesn't need be an exact match
    if (toDataType.StructDefinition == null)
    {
      return false;
    }

    // If we're assigning to an unnamed struct, the fields must be identical, not just assignable. We enforce this to ensure consistent behavior when taking the
    // common data type of a list of multiple unnamed structs (there should only be a common data type if all entries are identical in this case).
    if (toDataType.StructDefinition is UnnamedStructDefinitionAstNode)
    {
      // Unnamed structs themselves should never be const or upsampled, these properties can be on the fields
      Debug.Assert(toDataType.RuntimeMutability == RuntimeMutability.Variable);
      Debug.Assert(toDataType.UpsampleFactor == 1);
      return fromDataType.IsIdenticalTo(toDataType);
    }

    // All fields must individually be assignable and no extra fields can be provided. If we're assigning to a named struct containing fields with default
    // initializers, those fields do not have to be present.
    var fieldsRequiringAssignment = toDataType.StructDefinition.FlattenedFields.Where((field) => !field.HasDefaultValueExpression).ToHashSet();
    foreach (var fromStructField in fromDataType.StructDefinition.FlattenedFields)
    {
      var fromFieldDataType = fromDataType.GetModifiedFieldDataType(fromStructField);

      // A name match is good enough in properly written code but if the user specified duplicate fields, we should also do a type compatibility check. The
      // "from" field must be assignable to the "to" field. However, the field's data type itself may need to be modified based on the struct's modifiers so
      // we need to account for that.
      var toStructField = toDataType.StructDefinition.FlattenedFields.FirstOrDefault(
        (field) => field.Name == fromStructField.Name && fromFieldDataType.IsAssignableTo(toDataType.GetModifiedFieldDataType(field)));

      // The field must exist on the target
      if (toStructField == null)
      {
        return false;
      }

      fieldsRequiringAssignment.Remove(toStructField);
    }

    // If there are any fields left over, the struct as a whole is not assignable
    return fieldsRequiringAssignment.Count == 0;
  }
}