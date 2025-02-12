namespace Compiler.Parser.Nodes;

internal class SourceFileParseTreeNode : ParseTreeNode
{
  private SourceFileParseTreeNode(
    SourceLocation sourceLocation,
    ImportListParseTreeNode importList,
    InstrumentGlobalListParseTreeNode instrumentGlobalList,
    GlobalScopeParseTreeNode globalScope)
    : base(sourceLocation)
  {
    ImportList = importList;
    InstrumentGlobalList = instrumentGlobalList;
    GlobalScope = globalScope;
  }

  public ImportListParseTreeNode ImportList { get; }
  public InstrumentGlobalListParseTreeNode InstrumentGlobalList { get; }
  public GlobalScopeParseTreeNode GlobalScope { get; }

  public static SourceFileParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();

    var importList = ImportListParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(importList.SourceLocation);

    var instrumentGlobalList = InstrumentGlobalListParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(instrumentGlobalList.SourceLocation);

    var globalScope = GlobalScopeParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(globalScope.SourceLocation);

    return new(tokenSourceLocations.Merge(), importList, instrumentGlobalList, globalScope);
  }
}