using Compiler.Ast;
using Compiler.Parser.Nodes;
using System.Diagnostics;

namespace Compiler.AstBuilder;

internal static class DataTypeNodeExtensions
{
  public static AstDataType ToAstDataType(this DataTypeParseTreeNode dataType, ScopeAstNode scope, IReporting reporting)
  {
    AstDataType astDataType;
    if (dataType.PrimitiveType != null)
    {
      astDataType = new(dataType.RuntimeMutability, dataType.PrimitiveType.Value, dataType.UpsampleFactor ?? 1, dataType.IsArray);
    }
    else if (dataType.IsVoid)
    {
      astDataType = AstDataType.Void();
    }
    else
    {
      Debug.Assert(dataType.TypeName != null);
      var typeStructDefinition = NameResolver.TryGetStructByName(scope, dataType.TypeName);
      if (typeStructDefinition == null)
      {
        reporting.ResolveTypeNameError(dataType.TypeName);
        astDataType = AstDataType.Error();
      }
      else
      {
        astDataType = new(dataType.RuntimeMutability, typeStructDefinition, dataType.UpsampleFactor ?? 1, dataType.IsArray);
      }
    }

    return astDataType;
  }
}