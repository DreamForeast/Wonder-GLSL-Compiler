open Wonder_jest;

let _ =
  describe(
    "parse",
    () => {
      open Expect;
      open Expect.Operators;
      open Sinon;
      let sandbox = getSandboxDefaultVal();
      beforeEach(() => sandbox := createSandbox());
      describe(
        "parseSegment",
        () =>
          test(
            "parse segment",
            () =>
              Parse.parseSegment(
                "../src/basic.glsl",
                {|
         @top
         precision highp float;
         @end

         @define
         #import "common_vertex"
         @end



         @varDeclare

         varying vec2 v_mapCoord0;

         varying vec2 v_mapCoord1;
         @end

         @funcDeclare
         vec3 getDirectionLightDirByLightPos(vec3 lightPos);
         @end

         @funcDefine
         vec3 getDirectionLightDirByLightPos(vec3 lightPos){
             return lightPos - vec3(0.0);
         }
         @end

         @body
         gl_Position = u_pMatrix * u_vMatrix * mMatrix * vec4(a_position, 1.0);
         @end

         |}
              )
              |> StringTool.removeSegmentDataBlankNewLine
              |>
              expect == (
                          (
                            "basic",
                            ("top", "precision highp float;"),
                            ("define", {|#import "common_vertex"|}),
                            (
                              "varDeclare",
                              {|
varying vec2 v_mapCoord0;

varying vec2 v_mapCoord1;
    |}
                            ),
                            (
                              "funcDeclare",
                              {|
         vec3 getDirectionLightDirByLightPos(vec3 lightPos);
    |}
                            ),
                            (
                              "funcDefine",
                              {|
         vec3 getDirectionLightDirByLightPos(vec3 lightPos){
             return lightPos - vec3(0.0);
         }
                    |}
                            ),
                            (
                              "body",
                              {|
         gl_Position = u_pMatrix * u_vMatrix * mMatrix * vec4(a_position, 1.0);
                    |}
                            )
                          )
                          |> StringTool.removeSegmentDataBlankNewLine
                        )
          )
      );
      describe(
        "parseImport",
        () => {
          test(
            "support one glsl's segment import other glsl's segment",
            () => {
              let list = [
                (
                  "basic0",
                  ("top", ""),
                  ("define", {|
#import "basic1"
define A 1;
#import "basic2"
define B 2;
    |}),
                  (
                    "varDeclare",
                    {|
#import "basic2"

varying vec2 v_mapCoord0;

#import "basic1"
    |}
                  ),
                  ("funcDeclare", ""),
                  ("funcDefine", ""),
                  ("body", "")
                ),
                (
                  "basic1",
                  ("top", ""),
                  ("define", {|
define C 3;
#import "basic2"
    |}),
                  ("varDeclare", {|
varying vec2 v_mapCoord1;
    |}),
                  ("funcDeclare", ""),
                  ("funcDefine", ""),
                  ("body", "")
                ),
                (
                  "basic2",
                  ("top", ""),
                  ("define", {|
define D 4;
    |}),
                  ("varDeclare", {|
varying vec2 v_mapCoord2;
    |}),
                  ("funcDeclare", ""),
                  ("funcDefine", ""),
                  ("body", "")
                )
              ];
              StringTool.removeBlankNewLine(Parse.parseImport(list))
              |>
              expect == StringTool.removeBlankNewLine(
                          {|
|> set("basic0", _buildChunk("","define C 3;define D 4;    define A 1;define D 4;","varying vec2 v_mapCoord2;    varying vec2 v_mapCoord0;varying vec2 v_mapCoord1;","","","")
|> set("basic1", _buildChunk("","define C 3;define D 4;","","","","")
|> set("basic2", _buildChunk("","","","","","")
|}
                        )
            }
          );
          test(
            "check circular reference",
            () => {
              let list = [
                (
                  "basic0",
                  ("top", ""),
                  ("define", {|
#import "basic1"
define A 1;
    |}),
                  ("varDeclare", ""),
                  ("funcDeclare", ""),
                  ("funcDefine", ""),
                  ("body", "")
                ),
                (
                  "basic1",
                  ("top", ""),
                  ("define", {|
define C 3;
#import "basic2"
    |}),
                  ("varDeclare", ""),
                  ("funcDeclare", ""),
                  ("funcDefine", ""),
                  ("body", "")
                ),
                (
                  "basic2",
                  ("top", ""),
                  ("define", {|
#import "basic0"
    |}),
                  ("varDeclare", ""),
                  ("funcDeclare", ""),
                  ("funcDefine", ""),
                  ("body", "")
                )
              ];
              expect(() => Parse.parseImport(list) |> ignore)
              |> toThrowMessage(
                   "not allow circular reference(the reference path is basic0=>basic1=>basic2=>basic0)"
                 )
            }
          )
        }
      )
    }
  );