using Compiler.Lexer;

namespace Compiler.Parser.Nodes;

// Represents a type name which may be within a scope, e.g. Foo.Bar
internal class TypeNameParseTreeNode : ParseTreeNode
{
  private const string _component = "type name";

  private TypeNameParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<string> components)
    : base(sourceLocation)
    => Components = components;

  public IReadOnlyList<string> Components { get; }

  public static TypeNameParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();

    var components = new List<string>();

    {
      var component = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
      if (component == null)
      {
        context.Reporting.MalformedError(_component, location.NextToken(), "type name");
        location.RecoverFromError();
      }

      components.Add(component);
    }

    while (location.ConsumeIfNextTokenIs(TokenType.Period, tokenSourceLocations) != null)
    {
      var component = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
      if (component == null)
      {
        context.Reporting.MalformedError(_component, location.NextToken(), "scope alias");
        location.RecoverFromError();
      }

      components.Add(component);
    }

    return new(tokenSourceLocations.Merge(), components);
  }

  public string ToLanguageString()
    => string.Join('.', Components);
}