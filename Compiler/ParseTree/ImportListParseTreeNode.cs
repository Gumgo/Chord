using Compiler.Parsing;
using Compiler.Tokenization;

namespace Compiler.ParseTree;

internal class ImportListParseTreeNode : ParseTreeNode
{
  private ImportListParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<ImportParseTreeNode> imports)
    : base(sourceLocation)
    => Imports = imports;

  public IReadOnlyList<ImportParseTreeNode> Imports { get; }

  public static ImportListParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>() { SourceLocation.FromFile(location.File) };
    var imports = new List<ImportParseTreeNode>();
    while (ImportParseTreeNode.CanParse(location.NextToken()))
    {
      ErrorRecovery.Run(
        ErrorRecoveryBehavior.AllowOuterScopeRecovery,
        (previousToken, nextToken) => (previousToken?.IsEndOfLine ?? false)
          || previousToken?.TokenType == TokenType.Semicolon
          || nextToken.TokenType == TokenType.EndOfFile,
        () =>
        {
          var import = ImportParseTreeNode.Parse(context, location);
          tokenSourceLocations.Add(import.SourceLocation);
          imports.Add(import);
        });
    }

    return new(tokenSourceLocations.Merge(), imports);
  }
}