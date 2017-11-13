open Wonder_jest;

let _ =
  describe(
    "create shunk system file",
    () => {
      open Expect;
      open Expect.Operators;
      open Sinon;
      let sandbox = getSandboxDefaultVal();
      let destFilePath = ref("./shaderChunkSystem.re");
      beforeEach(() => sandbox := createSandbox());
      afterEach(
        () =>
          if (Node.Fs.existsSync(destFilePath^)) {
            Node.Fs.unlinkSync(destFilePath^)
          } else {
            ()
          }
      );
      test(
        "add private functions",
        () => {
          Create.createShunkSystemFile([||], destFilePath^, [@bs] (() => ()));
          Node.Fs.readFileSync(destFilePath^, `utf8)
          |> StringTool.removeBlankNewLine
          |> expect
          |> toContainString(
               {|
open Contract;

open ShaderChunkType;

let _getGLSLChunkMap = (state: StateDataType.state) => state.glslChunkData.chunkMap;

let getChunk = (name: string, state: StateDataType.state) =>
  state |> _getGLSLChunkMap |> WonderCommonlib.HashMapSystem.get(name) |> Js.Option.getExn;

let _buildChunk =
    (
      top: string,
      define: string,
      varDeclare: string,
      funcDeclare: string,
      funcDefine: string,
      body: string
    ) => {
  top,
  define,
  varDeclare,
  funcDeclare,
  funcDefine,
  body
};
|}
               |> StringTool.removeBlankNewLine
             )
        }
      );
      test(
        "test parsed glsl content",
        () => {
          Create.createShunkSystemFile(
            [|"./test/res/glsl1.glsl", "./test/res/glsl2.glsl"|],
            destFilePath^,
            [@bs] (() => ())
          );
          Node.Fs.readFileSync(destFilePath^, `utf8)
          |> StringTool.removeBlankNewLine
          |> expect
          |> toContainString(
               "|> set(\"glsl2\", _buildChunk({||},{|define B 2;|},{|varying vec2 v_mapCoord2;|},{|vec3 func2(vec3 lightPos);|},{|vec3 func2(vec3 lightPos){\n                       return vec3(0.5);\n                   }|},{|gl_FragColor = vec4(1.0,0.5,1.0,1.0);|}))\n                |> set(\"glsl1\", _buildChunk({|precision highp float;|},{|define B 2;|},{|varying vec2 v_mapCoord0;\n\n                   varying vec2 v_mapCoord1;|},{|vec3 func1(vec3 lightPos);|},{|vec3 func2(vec3 lightPos){\n                       return vec3(0.5);\n                   }\n                   vec3 func1(vec3 lightPos){\n                       return vec3(1.0);\n                   }|},{|gl_Position = u_pMatrix * u_vMatrix * mMatrix * vec4(a_position, 1.0);|}))"
               |> StringTool.removeBlankNewLine
             )
        }
      );
      test(
        "exec done func",
        () => {
          let done_ = createEmptyStubWithJsObjSandbox(sandbox);
          Create.createShunkSystemFile([|"./test/res/glsl2.glsl"|], destFilePath^, [@bs] done_);
          Node.Fs.readFileSync(destFilePath^, `utf8) |> ignore;
          done_ |> expect |> toCalledOnce
        }
      );
      test(
        "support pass glob path array",
        () => {
          let done_ = createEmptyStubWithJsObjSandbox(sandbox);
          Create.createShunkSystemFile([|"./test/res/glsl*"|], destFilePath^, [@bs] done_);
          let content =
            Node.Fs.readFileSync(destFilePath^, `utf8) |> StringTool.removeBlankNewLine;
          (content |> Js.String.includes("glsl1"), content |> Js.String.includes("glsl2"))
          |> expect == (true, true)
        }
      )
    }
  );