using Compiler.Lexer;
using Compiler.Types;

namespace Compiler.Parser.Nodes;

file static class ReportingExtensions
{
  public static void IllegalUpsampleFactorError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error(
      "IllegalUpsampleFactor",
      sourceLocation,
      $"Upsample factor not allowed on data type with '{RuntimeMutability.Constant.ToLanguageString()}' runtime mutability");

  public static void IllegalVoidDataTypeQualifierError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error(
      "IllegalVoidDataTypeQualifier",
      $"Illegal qualifier specified on '{VoidDataType.LanguageString}' data type");
}

internal class DataTypeParseTreeNode : ParseTreeNode
{
  private const string _component = "data type";

  private DataTypeParseTreeNode(
    SourceLocation sourceLocation,
    RuntimeMutability runtimeMutability,
    TypeNameParseTreeNode? typeName,
    PrimitiveType? primitiveType,
    bool isVoid,
    int? upsampleFactor,
    bool isArray)
    : base(sourceLocation)
  {
    RuntimeMutability = runtimeMutability;
    TypeName = typeName;
    PrimitiveType = primitiveType;
    IsVoid = isVoid;
    UpsampleFactor = upsampleFactor;
    IsArray = isArray;
  }

  public RuntimeMutability RuntimeMutability { get; }
  public TypeNameParseTreeNode? TypeName { get; }
  public PrimitiveType? PrimitiveType { get; }
  public bool IsVoid { get; }
  public int? UpsampleFactor { get; }
  public bool IsArray { get; }

  public static DataTypeParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();

    var runtimeMutability = location.ConsumeIfNextTokenIs(TokenType.KeywordConst, tokenSourceLocations) == null
      ? RuntimeMutability.Variable
      : RuntimeMutability.Constant;
    if (runtimeMutability == RuntimeMutability.Constant && location.ConsumeIfNextTokenIs(TokenType.QuestionMark, tokenSourceLocations) != null)
    {
      runtimeMutability = RuntimeMutability.DependentConstant;
    }

    var primitiveType = location.NextToken().TokenType.GetPrimitiveType();
    var isVoid = false;
    TypeNameParseTreeNode? typeName = null;
    if (primitiveType != null)
    {
      location.ConsumeNextToken(tokenSourceLocations);
    }
    else if (location.ConsumeIfNextTokenIs(TokenType.KeywordVoid, tokenSourceLocations) != null)
    {
      isVoid = true;
    }
    else
    {
      typeName = TypeNameParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(typeName.SourceLocation);
    }

    int? upsampleFactor = null;
    var atToken = location.ConsumeIfNextTokenIs(TokenType.At, tokenSourceLocations);
    if (atToken != null)
    {
      upsampleFactor = location.ConsumeIfNextTokenIs(TokenType.UpsampleFactor, tokenSourceLocations)?.UpsampleFactorValue;
      if (upsampleFactor == null)
      {
        context.Reporting.MalformedError(_component, location.NextToken(), TokenType.UpsampleFactor);
        location.RecoverFromError();
      }
    }

    if (atToken != null && runtimeMutability == RuntimeMutability.Constant)
    {
      context.Reporting.IllegalUpsampleFactorError(atToken.SourceLocation);
      upsampleFactor = null;
    }

    var isArray = false;
    if (location.ConsumeIfNextTokenIs(TokenType.LeftBracket, tokenSourceLocations) != null)
    {
      if (location.ConsumeIfNextTokenIs(TokenType.RightBracket, tokenSourceLocations) == null)
      {
        context.Reporting.MalformedError(_component, location.NextToken(), TokenType.RightBracket);
        location.RecoverFromError();
      }

      isArray = true;
    }

    if (isVoid && (runtimeMutability != RuntimeMutability.Variable || upsampleFactor != null || isArray))
    {
      context.Reporting.IllegalVoidDataTypeQualifierError(tokenSourceLocations.Merge());
      runtimeMutability = RuntimeMutability.Variable;
      upsampleFactor = null;
      isArray = false;
    }

    return new(tokenSourceLocations.Merge(), runtimeMutability, typeName, primitiveType, isVoid, upsampleFactor, isArray);
  }

  public string ToLanguageString()
  {
    var runtimeMutabilityString = RuntimeMutability.ToLanguageString();
    runtimeMutabilityString = runtimeMutabilityString == null
      ? string.Empty
      : $"{runtimeMutabilityString} ";
    var typeNameString = PrimitiveType?.ToLanguageString()
      ?? (IsVoid ? VoidDataType.LanguageString : null)
      ?? TypeName?.ToLanguageString()
      ?? throw new InvalidOperationException("No type name specified");
    var upsampleFactorString = UpsampleFactor == null
      ? string.Empty
      : $"@{UpsampleFactor}x";
    var arrayString = IsArray ? "[]" : string.Empty;
    return $"{runtimeMutabilityString}{typeNameString}{upsampleFactorString}{arrayString}";
  }
}