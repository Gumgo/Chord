using Compiler.Types;

namespace Compiler.Program.ProgramGraphNodes;

internal class ConstantProgramGraphNode : IProcessorProgramGraphNode
{
  public ConstantProgramGraphNode(float value)
  {
    Output = new OutputProgramGraphNode(this, new(PrimitiveType.Float, null, false), 0);
    Value = value;
  }

  public ConstantProgramGraphNode(double value)
  {
    Output = new OutputProgramGraphNode(this, new(PrimitiveType.Double, null, false), 0);
    Value = value;
  }

  public ConstantProgramGraphNode(int value)
  {
    Output = new OutputProgramGraphNode(this, new(PrimitiveType.Int, null, false), 0);
    Value = value;
  }

  public ConstantProgramGraphNode(bool value)
  {
    Output = new OutputProgramGraphNode(this, new(PrimitiveType.Bool, null, false), 0);
    Value = value;
  }

  public ConstantProgramGraphNode(string value)
  {
    Output = new OutputProgramGraphNode(this, new(PrimitiveType.String, null, false), 0);
    Value = value;
  }

  public IOutputProgramGraphNode Output { get; }
  public object Value { get; }

  public float FloatValue => (float)Value;
  public double DoubleValue => (double)Value;
  public int IntValue => (int)Value;
  public bool BoolValue => (bool)Value;
  public string StringValue => (string)Value;
}