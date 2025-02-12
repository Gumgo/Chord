using Compiler.Ast;
using Compiler.Types;

namespace Compiler.NativeLibrary;

internal class NativeModuleParameter(ModuleParameterDirection direction, string name, AstDataType dataType)
{
  public ModuleParameterDirection Direction { get; } = direction;
  public string Name { get; } = name;
  public AstDataType DataType { get; } = dataType;

  public override bool Equals(object? obj)
    => obj is NativeModuleParameter other && Direction == other.Direction && Name == other.Name && DataType.IsIdenticalTo(other.DataType);

  public override int GetHashCode()
    => HashCode.Combine(Direction, Name, DataType);
}

internal class NativeModuleSignature(string name, int? returnParameterIndex, params NativeModuleParameter[] parameters)
{
  public string Name { get; } = name;
  public IReadOnlyList<NativeModuleParameter> Parameters { get; } = parameters;
  public int? ReturnParameterIndex { get; } = returnParameterIndex;

  public override bool Equals(object? obj)
    => obj is NativeModuleSignature other
    && Name == other.Name
    && Parameters.Count == other.Parameters.Count
    && Parameters.SequenceEqual(other.Parameters)
    && ReturnParameterIndex == other.ReturnParameterIndex;

  public override int GetHashCode()
    => HashCode.Combine(Name, Parameters, ReturnParameterIndex);
}