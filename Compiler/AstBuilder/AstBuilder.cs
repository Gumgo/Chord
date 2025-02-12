using Compiler.Ast;

namespace Compiler.AstBuilder;

internal class AstBuilder(AstBuilderContext context)
{
  public IReadOnlyDictionary<string, ScopeAstNode> BuildNativeLibraryAsts(IReadOnlyList<SourceFile> sourceFiles)
  {
    var allNativeImports = sourceFiles.SelectMany((sourceFile) => sourceFile.NativeImports.Select((nativeImport) => nativeImport.ResolvedPath)).ToHashSet();
    var nativeLibraryAsts = new Dictionary<string, ScopeAstNode>();

    foreach (var nativeImport in allNativeImports)
    {
      if (!context.NativeLibraryRegistry.TryGetNativeLibrary(nativeImport, out var nativeLibrary))
      {
        throw new ArgumentException("Native import not found");
      }

      var sourceLocation = SourceLocation.FromNativeLibrary(nativeImport);
      var nativeLibraryScope = new ScopeAstNode(sourceLocation, null, null);
      foreach (var nativeModule in nativeLibrary.Modules)
      {
        var nativeModuleDefinition = new NativeModuleDefinitionAstNode(nativeLibrary, nativeModule, nativeLibraryScope);

        // This scope will never be used, it just exists for the parameter values
        var moduleScope = new ScopeAstNode(sourceLocation, nativeLibraryScope, nativeModuleDefinition);

        nativeModuleDefinition.InitializeParameters();
        for (var parameterIndex = 0; parameterIndex < nativeModule.Signature.Parameters.Count; parameterIndex++)
        {
          if (parameterIndex != nativeModule.Signature.ReturnParameterIndex)
          {
            var parameter = nativeModule.Signature.Parameters[parameterIndex];

            var parameterValueDefinition = new ValueDefinitionAstNode(sourceLocation, moduleScope, parameter.Name);
            parameterValueDefinition.InitializeAssignmentExpression(null);

            var parameterAstNode = new ModuleParameterAstNode(sourceLocation, parameter.Direction, parameter.Name, parameter.DataType);
            parameterAstNode.InitializeDefaultValueExpression(null);
            parameterAstNode.InitializeValueDefinition(parameterValueDefinition);

            nativeModuleDefinition.AddParameter(parameterAstNode);
          }
        }

        if (nativeModule.Signature.ReturnParameterIndex != null)
        {
          nativeModuleDefinition.InitializeReturnDataType(nativeModule.Signature.Parameters[nativeModule.Signature.ReturnParameterIndex.Value].DataType);
        }

        nativeLibraryScope.AddScopeItem(nativeModuleDefinition);
      }

      nativeLibraryAsts.Add(nativeImport, nativeLibraryScope);
    }

    return nativeLibraryAsts;
  }

  public void BuildAsts(IReadOnlyList<SourceFile> sourceFiles, IReadOnlyDictionary<string, ScopeAstNode> nativeLibraryAsts)
  {
    var defaultValueExpressionResolver = new DefaultValueExpressionResolver(context);
    var globalScopeBuilder = new GlobalScopeBuilder(context);
    var valueBuilder = new ValueBuilder(context, defaultValueExpressionResolver);
    var structBuilder = new StructBuilder(context, defaultValueExpressionResolver);
    var moduleBuilder = new ModuleBuilder(context, defaultValueExpressionResolver);

    foreach (var sourceFile in sourceFiles)
    {
      sourceFile.Ast = globalScopeBuilder.BuildGlobalScope(sourceFile);
    }

    var sourceFilesDict = sourceFiles.ToDictionary((v) => v.Path);
    foreach (var sourceFile in sourceFiles)
    {
      globalScopeBuilder.ImportIntoGlobalScope(sourceFile, sourceFilesDict, nativeLibraryAsts);
    }

    foreach (var sourceFile in sourceFiles)
    {
      valueBuilder.BuildGlobalValues(sourceFile);
    }

    structBuilder.BuildStructs(sourceFiles);

    foreach (var sourceFile in sourceFiles)
    {
      moduleBuilder.BuildModuleSignatures(sourceFile);
    }

    foreach (var sourceFile in sourceFiles)
    {
      globalScopeBuilder.DetectModuleNameConflicts(sourceFile);
    }

    foreach (var sourceFile in sourceFiles)
    {
      structBuilder.BuildStructFieldDefaultValueExpressions(sourceFile);
      moduleBuilder.BuildModuleParameterDefaultValueExpressions(sourceFile);
      valueBuilder.BuildGlobalValueExpressions(sourceFile);
    }

    foreach (var sourceFile in sourceFiles)
    {
      moduleBuilder.BuildModuleBodies(sourceFile);
    }
  }
}