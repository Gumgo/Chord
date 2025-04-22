using Compiler.ParseTree;
using Compiler.Tokenization;

namespace Compiler.Parsing;

internal class ParserContext
{
  public required IReporting Reporting { get; init; }
}

internal class Parser(ParserContext context)
{
  public SourceFileParseTreeNode Process(string file, IReadOnlyList<Token> tokens)
    => SourceFileParseTreeNode.Parse(context, new(file, tokens));
}