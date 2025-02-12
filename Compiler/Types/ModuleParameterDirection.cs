using Compiler.Utilities;

namespace Compiler.Types;

internal enum ModuleParameterDirection
{
  In,
  Out,
}

internal static class ModuleParameterDirectionExtensions
{
  public static string ToLanguageString(this ModuleParameterDirection moduleParameterDirection)
    => moduleParameterDirection switch
    {
      ModuleParameterDirection.In => "in",
      ModuleParameterDirection.Out => "out",
      _ => throw UnhandledEnumValueException.Create(moduleParameterDirection),
    };
}