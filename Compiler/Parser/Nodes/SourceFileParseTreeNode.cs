namespace Compiler.Parser.Nodes;

internal class SourceFileParseTreeNode : ParseTreeNode
{
  private SourceFileParseTreeNode(
    SourceLocation sourceLocation,
    ImportListParseTreeNode importList,
    InstrumentPropertyListParseTreeNode instrumentPropertyList,
    GlobalScopeParseTreeNode globalScope)
    : base(sourceLocation)
  {
    ImportList = importList;
    InstrumentPropertyList = instrumentPropertyList;
    GlobalScope = globalScope;
  }

  public ImportListParseTreeNode ImportList { get; }
  public InstrumentPropertyListParseTreeNode InstrumentPropertyList { get; }
  public GlobalScopeParseTreeNode GlobalScope { get; }

  public static SourceFileParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();

    var importList = ImportListParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(importList.SourceLocation);

    var instrumentPropertyList = InstrumentPropertyListParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(instrumentPropertyList.SourceLocation);

    var globalScope = GlobalScopeParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(globalScope.SourceLocation);

    return new(tokenSourceLocations.Merge(), importList, instrumentPropertyList, globalScope);
  }
}