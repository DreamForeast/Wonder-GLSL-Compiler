let _functionContent = {|
open Contract;

open ShaderChunkType;

let _getGLSLChunkMap = (state: StateDataType.state) => state.glslChunkData.chunkMap;

let getChunk = (name: string, state: StateDataType.state) =>
  state |> _getGLSLChunkMap |> HashMapSystem.get(name) |> Js.Option.getExn;

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

/* todo auto write by glsl compiler */
let initData = () =>
|};

let _buildInitDataContent = (glslContent: string) => {j|
  HashMapSystem.{
    chunkMap:
      createEmpty()
      $glslContent
  };
|j};

let _buildShunkSystemFileContent = (glslContent) =>
  _functionContent ++ _buildInitDataContent(glslContent);

let _writeToShunkSystemFile = (destFilePath, doneFunc, content) => {
  Node.Fs.writeFileSync(~filename=destFilePath, ~text=content);
  [@bs] doneFunc() |> ignore
};

let createShunkSystemFile = (glslPathArr: array(string), destFilePath: string, doneFunc) =>
  /* let content = _functionContent; */
  /* let glslContent = ref(""); */
  glslPathArr
  |> Js.Array.reduce(
       (glslContentList, glslPath) =>
         Glob.sync(glslPath)
         |> Js.Array.reduce
              /* (glslContent: string, actualGlslPath) =>
                   glslContent ++ (readFileSync(actualGlslPath, `utf8) |> Parse.parse(actualGlslPath)),
                 glslContent */
              (
                (glslContentList, actualGlslPath) => [
                  Node.Fs.readFileSync(actualGlslPath, `utf8) |> Parse.parseSegment(actualGlslPath),
                  ...glslContentList
                ],
                glslContentList
              ),
       /* let a = readFileSync(glslPath, `utf8) |> String.length;
          [@bs] doneFunc() |> ignore;
          () */
       []
     )
  |> Parse.parseImport
  |> _buildShunkSystemFileContent
  |> _writeToShunkSystemFile(destFilePath, doneFunc);