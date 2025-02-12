using Compiler.Types;
using Compiler.Utilities;

namespace Compiler.Ast;

internal abstract class ModuleDefinitionAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope, string name)
  : AstNode(sourceLocation), IScopeItem, INamedAstNode
{
  private readonly InitializableValue<List<ModuleParameterAstNode>> _parameters = new();
  private readonly InitializableValue<AstDataType> _returnDataType = new();

  public ScopeAstNode ContainingScope => containingScope;
  public string Name => name;

  public IReadOnlyList<ModuleParameterAstNode> Parameters => _parameters.Value;
  public AstDataType ReturnDataType => _returnDataType.Value;

  public bool IsDependentConstant => Parameters.Any((parameter) => parameter.DataType.RuntimeMutability == RuntimeMutability.DependentConstant)
    || ReturnDataType.RuntimeMutability == RuntimeMutability.DependentConstant;

  public void InitializeParameters()
    => _parameters.Initialize([]);

  public void AddParameter(ModuleParameterAstNode parameter)
    => _parameters.Value.Add(parameter);

  public void InitializeReturnDataType(AstDataType returnDataType)
    => _returnDataType.Initialize(returnDataType);
}