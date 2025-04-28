<a id="readme-top"></a>

<div align="center">
  <!-- logo will go here
    <a href="https://github.com/Gumgo/Chord">
      <img src="images/logo.png" alt="Logo" width="80" height="80">
    </a>
  -->

  <!-- When we have a logo, change this to h3 -->
  <h1 align="center">Chord</h1>

  <p align="center">
    An extendable audio synthesis language for quickly building high-performance audio plugins
    <!-- Docs will go here
      <br />
      <a href="https://github.com/Gumgo/Chord/docs"><strong>Explore the docs »</strong></a>
    -->
    <br />
    <br />
    <!-- Enable these, eventually
      <a href="https://github.com/Gumgo/Chord/demo">View Demo</a>
      &middot;
      <a href="https://github.com/Gumgo/Chord/issues/new?labels=bug&template=bug-report---.md">Report Bug</a>
      &middot;
      <a href="https://github.com/Gumgo/Chord/issues/new?labels=enhancement&template=feature-request---.md">Request Feature</a>
    -->
  </p>
</div>

## About The Project

Chord is a simple audio synthesis language intended to make it easy to quickly write and iterate on audio plugins, such as VST instruments or effects. Well, at least, it will be, as it's still currently being written. I originally started the project back in 2015 (here is the [original repository](https://github.com/Gumgo/WaveLang)). Since then, I spent seven years working at an audio R&D/tech company where I gained a much better understanding of audio synthesis and manipulation and built up a big list of ideas for an improved version of the language. I'm hoping to have an initial working version done by the end of this year.

Chord is a statically-typed language with a simple and familiar syntax based mostly off of C# and a bit of TypeScript. An audio plugin simply consists of a script file which gets transformed into an optimized DAG of low-level audio buffer operations when compiled. At runtime, this DAG gets loaded and executed via a multithreaded job scheduler.

A number of simple low-level operations are built into the language, such as the ability to perform basic arithmetic or transcendental functions on a signal. Fundamental audio operations are also included, such as delaying a signal or running IIR and FIR filters. These built-in operations can be combined and wrapped up into script-defined "modules" (which are just Chord's version of functions, but I chose the name "module" to go with the idea of modular synthesis) for easy reuse. Modules can then be exported out of one script file and imported into another so that plugin scripts don't become too long and disorganized.

Chord is intended to be easily extendable, so even the low-level operations (known as "native modules") aren't hardcoded into the compiler or the runtime. Instead, they are provided via "native libraries": DLLs which expose a list of native modules using a C API.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Some examples

Here are some simple examples of Chord's syntax. I'll eventually write more complete documentation but that's not very useful until the language is functional.

Declaring variables uses a similar syntax to TypeScript.

```
val x = 1.0f;
val y: double;
y = 0.0;
val str = "Chord";
```

Structs can be used to group related values.

```
struct Foo
{
  bar: double = 1.0;
  baz: const string;
}

val x = { bar = 1.0, baz = "str" } as Foo;
val y: Foo = { baz = "str" }; // bar is not declared because it has a default initializer

```

Declaring a variable as "const" means that its value must be fully-resolved at compile-time. This is useful to make sure that unnecessary audio processing operations aren't happening at runtime. Also, certain audio operations require values that don't ever change (such as the sample count in a fixed-length delay operation).

```
val delaySamples: const int = 100;
Delay(signal, delaySamples);
```

This script-defined module produces a simple "echo" effect:

```
module Echo(signal: float, delaySampleCount: const int, decay: const float): float
{
  return signal + Delay(signal, delaySampleCount) * decay;
}
```

We can pass in an array to turn this into a multi-tap echo:

```
module MultiTapEcho(signal: float, delaySampleCounts: const int[], decay: const float): float
{
  val result = signal;
  val nextDecay = decay;
  for (val delaySampleCount in delaySampleCounts)
  {
    result += Delay(signal, delaySampleCount) * nextDecay;
	nextDecay *= decay;
  }

  return result;
}
```

If a module's parameters and return value are declared as `const?`, it means that the module will be treated as `const` only when all of its inputs are also `const`.

```
module SomeComplexCalculation(x: const? float, y: const? float, z: const? float): const? float
{
  return <some complex math goes here>;
}

// This call is guaranteed to get resolved at compile-time
val a: const float = SomeComplexCalculation(1.0f, 2.0f, 3.0f);

// This will get executed at runtime (unless foo, bar, and baz do happen to be constant!)
val b = SomeComplexCalculation(foo, bar, baz);
```

Band-limited resampling can be used to process signals at a higher sample rate.

```
// atan will introduce aliasing as it is non-linear so upsample to reduce these artifacts. The data type of xUp is float@4x.
val xUp = Upsample4x(x);

// Perform the soft-clipping at the higher sample rate
val xUpSoftClipped = math.Atan(x) / (math.FloatConstants.Pi * 0.5f);

// Downsample back to our base sample rate
val xSoftClipped = Downsample4x(xUpSoftClipped);
```

Some operations may introduce latency. Chord automatically handles latency compensation to make sure that signals are always aligned.

```
module FilterWithLatency(x: float): float
{
  val coefficients: const float[] = [1.0f, -2.0f, 3.0f, -2.0f, 1.0f];
  val latency: const int = (coefficients.length - 1) / 2;
  return AddLatency(filters.Fir(x, coefficients), latency);
}

val a = FilterWithLatency(x);
val b = 1.0f;

// b will automatically be delayed by 2 samples to align with a before the addition operation
val c = a + b;
```

Here is an example of a complete program which sums all input channels into a single (mono) output channel. This example uses the "effect" entry point which runs once on all input audio channels. A "voice" entry point can also be defined which runs once for each active voice, where voices are triggered by e.g. pressing a key on a MIDI keyboard.

```
import math;

// Declares EffectMain as the "effect" entry point which will be activated if any input channel level exceeds a threshold of 0.0001
#effect EffectMain threshold 0.0001f

module EffectMain(inputChannels: float[], out outputChannels: float[]): bool
{
  val anyAudio = false;
  val outputChannel = 0.0f;

  // Sum all of our input channels and detect if any of them contain audio
  for (val inputChannel in inputChannels)
  {
    outputChannel += inputChannel;
    anyAudio |= math.Abs(inputChannel) >= 0.0001f;
  }

  // Write out summed result as a single output channel
  outputChannels = [outputChannel /= inputChannels.length as float];

  // If all input channels are silent, we will return false which will disable effect processing
  return anyAudio;
}
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## License

Distributed under the MIT license. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>
