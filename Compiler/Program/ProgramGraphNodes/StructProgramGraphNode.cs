namespace Compiler.Program.ProgramGraphNodes;

internal class StructProgramGraphNode : IProcessorProgramGraphNode
{
  public StructProgramGraphNode(IReadOnlyDictionary<string, IOutputProgramGraphNode> fields)
  {
    Fields = fields
      .Select((v) => KeyValuePair.Create(v.Key, (IInputProgramGraphNode)new InputProgramGraphNode(this) { Connection = v.Value }))
      .ToDictionary();
    Output = new OutputProgramGraphNode(this, new(null, null, false), 0);
  }

  public IReadOnlyDictionary<string, IInputProgramGraphNode> Fields { get; }
  public IOutputProgramGraphNode Output { get; }
}