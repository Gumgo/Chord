using Compiler.Parsing;
using Compiler.Tokenization;
using System.Diagnostics;

namespace Compiler.ParseTree;

internal class ValueParseTreeNode : ParseTreeNode
{
  private const string _component = "value declaration";

  private ValueParseTreeNode(SourceLocation sourceLocation, string name, DataTypeParseTreeNode? dataType)
    : base(sourceLocation)
  {
    Name = name;
    DataType = dataType;
  }

  public string Name { get; }
  public DataTypeParseTreeNode? DataType { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.KeywordVal;

  public static ValueParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    var name = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
    if (name == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), "value name");
      location.RecoverFromError();
    }

    DataTypeParseTreeNode? dataType = null;
    if (location.ConsumeIfNextTokenIs(TokenType.Colon, tokenSourceLocations) != null)
    {
      dataType = DataTypeParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(dataType.SourceLocation);
    }

    return new(tokenSourceLocations.Merge(), name, dataType);
  }
}