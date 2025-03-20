using Compiler.Types;

namespace Compiler.Program.ProgramGraphNodes;

internal class ArrayProgramGraphNode : IProcessorProgramGraphNode
{
  public ArrayProgramGraphNode(PrimitiveType? primitiveType, IReadOnlyList<IOutputProgramGraphNode> elements)
  {
    int? upsampleFactor = null;
    foreach (var element in elements)
    {
      if (element.DataType.PrimitiveType != primitiveType)
      {
        throw new ArgumentException("Inconsistent array program stage graph node element primitive types");
      }

      if (upsampleFactor == null)
      {
        upsampleFactor ??= element.DataType.UpsampleFactor;
      }
      else if (element.DataType.UpsampleFactor != null)
      {
        if (upsampleFactor.Value != element.DataType.UpsampleFactor.Value)
        {
          throw new ArgumentException("Inconsistent array program stage graph node element upsample factors");
        }
      }

      if (element.DataType.IsArray)
      {
        throw new ArgumentException("Cannot create array program stage graph node using elements which are themselves arrays");
      }
    }

    var elementDataType = new ProgramDataType(primitiveType, upsampleFactor, false);
    var arrayDataType = new ProgramDataType(primitiveType, upsampleFactor, true);

    PrimitiveType = primitiveType;
    Elements = elements.Select((v) => new InputProgramGraphNode(this) { Connection = v }).ToArray();
    Output = new OutputProgramGraphNode(this, elementDataType, 0);
  }

  public PrimitiveType? PrimitiveType { get; }
  public IReadOnlyList<IInputProgramGraphNode> Elements { get; }
  public IOutputProgramGraphNode Output { get; }
}