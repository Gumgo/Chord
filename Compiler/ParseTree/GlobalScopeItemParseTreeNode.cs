using Compiler.Parsing;
using Compiler.Tokenization;
using System.Diagnostics;

namespace Compiler.ParseTree;

internal class GlobalScopeItemParseTreeNode : ParseTreeNode
{
  private GlobalScopeItemParseTreeNode(SourceLocation sourceLocation, bool isExported, ValueDefinitionParseTreeNode valueDefinition)
    : base(sourceLocation)
  {
    IsExported = isExported;
    ValueDefinition = valueDefinition;
  }

  private GlobalScopeItemParseTreeNode(SourceLocation sourceLocation, bool isExported, StructDefinitionParseTreeNode structDefinition)
    : base(sourceLocation)
  {
    IsExported = isExported;
    StructDefinition = structDefinition;
  }

  private GlobalScopeItemParseTreeNode(SourceLocation sourceLocation, bool isExported, ModuleDefinitionParseTreeNode moduleDefinition)
    : base(sourceLocation)
  {
    IsExported = isExported;
    ModuleDefinition = moduleDefinition;
  }

  public bool IsExported { get; }
  public ValueDefinitionParseTreeNode? ValueDefinition { get; }
  public StructDefinitionParseTreeNode? StructDefinition { get; }
  public ModuleDefinitionParseTreeNode? ModuleDefinition { get; }

  public static bool CanParse(Token nextToken)
    => ValueDefinitionParseTreeNode.CanParse(nextToken)
    || StructDefinitionParseTreeNode.CanParse(nextToken)
    || ModuleDefinitionParseTreeNode.CanParse(nextToken);

  public static GlobalScopeItemParseTreeNode Parse(ParserContext context, ParserLocation location, Token? exportToken)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    if (exportToken != null)
    {
      Debug.Assert(exportToken.TokenType == TokenType.KeywordExport);
      tokenSourceLocations.Add(exportToken.SourceLocation);
    }

    var isExported = exportToken != null;

    if (ValueDefinitionParseTreeNode.CanParse(location.NextToken()))
    {
      var valueDefinition = ValueDefinitionParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(valueDefinition.SourceLocation);
      return new(tokenSourceLocations.Merge(), isExported, valueDefinition);
    }
    else if (StructDefinitionParseTreeNode.CanParse(location.NextToken()))
    {
      var structDefinition = StructDefinitionParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(structDefinition.SourceLocation);
      return new(tokenSourceLocations.Merge(), isExported, structDefinition);
    }
    else if (ModuleDefinitionParseTreeNode.CanParse(location.NextToken()))
    {
      var moduleDefinition = ModuleDefinitionParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(moduleDefinition.SourceLocation);
      return new(tokenSourceLocations.Merge(), isExported, moduleDefinition);
    }

    Debug.Assert(false);
    return null;
  }
}