using Compiler.Lexer;
using Compiler.Parser.Nodes;

namespace Compiler.Parser;

internal class ParserContext
{
  public required IReporting Reporting { get; init; }
}

// !!! organization: subdivide namespace hierarchy into the following:
// - ParseTree
// - Parser
// - Token
// - Lexer
internal class Parser(ParserContext context)
{
  public SourceFileParseTreeNode Process(string file, IReadOnlyList<Token> tokens)
    => SourceFileParseTreeNode.Parse(context, new(file, tokens));
}