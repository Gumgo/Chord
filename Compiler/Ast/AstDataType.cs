using Compiler.Types;

namespace Compiler.Ast;

internal class AstDataType
{
  public AstDataType(RuntimeMutability runtimeMutability, PrimitiveType primitiveType, int upsampleFactor, bool isArray)
  {
    RuntimeMutability = runtimeMutability;
    PrimitiveType = primitiveType;
    UpsampleFactor = upsampleFactor;
    IsArray = isArray;
  }

  public AstDataType(RuntimeMutability runtimeMutability, StructDefinitionAstNode structDefinition, int upsampleFactor, bool isArray)
  {
    RuntimeMutability = runtimeMutability;
    StructDefinition = structDefinition;
    UpsampleFactor = upsampleFactor;
    IsArray = isArray;
  }

  private AstDataType(RuntimeMutability runtimeMutability, int upsampleFactor, bool isArray)
  {
    RuntimeMutability = runtimeMutability;
    UpsampleFactor = upsampleFactor;
    IsArray = isArray;
  }

  public RuntimeMutability RuntimeMutability { get; }
  public PrimitiveType? PrimitiveType { get; }
  public StructDefinitionAstNode? StructDefinition { get; }
  public int UpsampleFactor { get; }
  public bool IsArray { get; }

  public bool IsEmptyArray { get; private init; }
  public bool IsModule { get; private init; }
  public bool IsScope { get; private init; }
  public bool IsError { get; private init; }

  public static AstDataType EmptyArray()
    => new(RuntimeMutability.Constant, 1, true) { IsEmptyArray = true };

  public static AstDataType Module()
    => new(RuntimeMutability.Constant, 1, false) { IsModule = true };

  public static AstDataType Scope()
    => new(RuntimeMutability.Constant, 1, false) { IsScope = true };

  public static AstDataType Void()
    => new(RuntimeMutability.Variable, Types.PrimitiveType.Void, 1, false);

  public static AstDataType Error()
    => new(RuntimeMutability.Constant, 1, true) { IsError = true };

  public bool IsLegal()
  {
    if (IsError)
    {
      // We already flag errors when they're detected, don't double-report them
      return true;
    }

    // Unnamed struct data types should always be variable runtime mutability and non-upsampled (the individual fields can vary these properties)
    if (StructDefinition is UnnamedStructDefinitionAstNode && (RuntimeMutability != RuntimeMutability.Variable || UpsampleFactor != 1))
    {
      return false;
    }

    if (RuntimeMutability == RuntimeMutability.Constant && UpsampleFactor != 1)
    {
      return false;
    }

    if (PrimitiveType == Types.PrimitiveType.Void)
    {
      return RuntimeMutability == RuntimeMutability.Variable && UpsampleFactor == 1 && !IsArray;
    }

    if (PrimitiveType != null && !PrimitiveType.Value.SupportsVariableRuntimeMutability())
    {
      return RuntimeMutability == RuntimeMutability.Constant && UpsampleFactor == 1;
    }

    return true;
  }

  public bool IsLegalImplicitValueType()
    => IsLegal() && !IsModule && !IsScope && PrimitiveType != Types.PrimitiveType.Void;

  public bool IsLegalValueType()
    => IsLegal() && !IsModule && !IsScope && !IsEmptyArray && PrimitiveType != Types.PrimitiveType.Void;

  public bool IsLegalParameterType()
    => IsLegalValueType();

  public bool IsLegalReturnType()
    => IsLegal() && !IsModule && !IsScope && !IsEmptyArray;

  public bool IsLegalFieldType()
    => IsLegalValueType() && RuntimeMutability != RuntimeMutability.DependentConstant;

  public string ToLanguageString()
  {
    if (IsEmptyArray)
    {
      return "<empty-array>";
    }

    if (IsModule)
    {
      return "<module>";
    }

    if (IsScope)
    {
      return "<scope>";
    }

    if (IsError)
    {
      return "<error>";
    }

    var runtimeMutabilityString = RuntimeMutability.ToLanguageString();
    runtimeMutabilityString = runtimeMutabilityString == null
      ? string.Empty
      : $"{runtimeMutabilityString} ";

    string typeNameString;
    if (PrimitiveType != null)
    {
      typeNameString = PrimitiveType.Value.ToLanguageString();
    }
    else if (StructDefinition != null)
    {
      typeNameString = StructDefinition is NamedStructDefinitionAstNode namedStructDefinition
        ? namedStructDefinition.Name
        : "<unnamed-struct>";
    }
    else
    {
      throw new InvalidOperationException("No type name specified");
    }

    var upsampleFactorString = UpsampleFactor == 1
      ? string.Empty
      : $"@{UpsampleFactor}x";
    var arrayString = IsArray ? "[]" : string.Empty;

    return $"{runtimeMutabilityString}{typeNameString}{upsampleFactorString}{arrayString}";
  }

  public override bool Equals(object? obj)
  {
    if (obj is not AstDataType other)
    {
      return false;
    }

    if (ReferenceEquals(this, other) || (IsError && other.IsError))
    {
      return true;
    }

    return this.IsIdenticalTo(other);
  }

  public override int GetHashCode()
  {
    var hashCode = HashCode.Combine(RuntimeMutability, PrimitiveType, UpsampleFactor, IsArray);

    if (StructDefinition is NamedStructDefinitionAstNode namedStructDefinition)
    {
      hashCode ^= namedStructDefinition.GetHashCode();
    }
    else if (StructDefinition is UnnamedStructDefinitionAstNode unnamedStructDefinition)
    {
      // If StructDefinition is an UnnamedStructDefinitionAstNode, we would need to process all fields (recursively) to determine a truly unique hash code. For
      // now, just hashing the field count is probably good enough (I don't even think GetHashCode() is ever even used, it's just implemented for completeness).
      hashCode ^= unnamedStructDefinition.Fields.Count.GetHashCode();
    }

    return hashCode;
  }
}