using Compiler.Ast;
using Compiler.Program.ProgramGraphNodes;

namespace Compiler.ProgramGraphBuilding;

internal class BuildGraphExpressionResult
{
  public required IOutputProgramGraphNode? Node { get; init; }

  // If this is a reference such as foo.bar[3], these properties hold the root value declaration and the chain of input nodes leading to the final value
  public required ValueDefinitionAstNode? ValueDefinition { get; init; }
  public required IReadOnlyList<IInputProgramGraphNode> ReferenceNodes { get; init; }
}