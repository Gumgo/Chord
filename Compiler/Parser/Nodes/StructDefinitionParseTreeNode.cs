using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes;

internal class StructDefinitionParseTreeNode : ParseTreeNode
{
  private const string _component = "struct definition";

  private StructDefinitionParseTreeNode(
    SourceLocation sourceLocation,
    string name,
    IReadOnlyList<TypeNameParseTreeNode> baseTypeNames,
    IReadOnlyList<StructFieldParseTreeNode> fields)
    : base(sourceLocation)
  {
    Name = name;
    BaseTypeNames = baseTypeNames;
    Fields = fields;
  }

  public string Name { get; }
  public IReadOnlyList<TypeNameParseTreeNode> BaseTypeNames { get; }
  public IReadOnlyList<StructFieldParseTreeNode> Fields { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.KeywordStruct;

  public static StructDefinitionParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    var name = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
    if (name == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), "struct name");
      location.RecoverFromError();
    }

    var baseTypeNames = new List<TypeNameParseTreeNode>();
    if (location.ConsumeIfNextTokenIs(TokenType.Colon, tokenSourceLocations) != null)
    {
      ParserUtilities.ParseCommaSeparatedList(
        context,
        location,
        tokenSourceLocations,
        _component,
        TokenType.LeftBrace,
        () => baseTypeNames.Add(TypeNameParseTreeNode.Parse(context, location)));
    }
    else if (location.ConsumeIfNextTokenIs(TokenType.LeftBrace, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.LeftBrace);
      location.RecoverFromError();
    }

    var fields = new List<StructFieldParseTreeNode>();
    while (location.ConsumeIfNextTokenIs(TokenType.RightBrace, tokenSourceLocations) == null)
    {
      if (location.NextToken().TokenType == TokenType.EndOfFile)
      {
        context.Reporting.MalformedError(_component, location.NextToken());
        location.RecoverFromError();
      }

      ErrorRecovery.Run(
        ErrorRecoveryBehavior.BlockOuterScopeRecovery,
        (previousToken, nextToken) => (previousToken?.IsEndOfLine ?? false)
          || previousToken?.TokenType == TokenType.Semicolon
          || nextToken.TokenType == TokenType.RightBrace
          || nextToken.TokenType == TokenType.EndOfFile,
        () =>
        {
          var structField = StructFieldParseTreeNode.Parse(context, location);
          tokenSourceLocations.Add(structField.SourceLocation);
          fields.Add(structField);
        });
    }

    return new(tokenSourceLocations.Merge(), name, baseTypeNames, fields);
  }
}