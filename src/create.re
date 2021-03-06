let _functionContent = {|
  open ShaderChunkType;

  open StateDataMainType;

  let _getGLSLChunkMap = ({chunkMap}) => chunkMap;

  let getChunk = (name: string, glslChunkRecord) =>
    glslChunkRecord
    |> _getGLSLChunkMap
    |> WonderCommonlib.MutableHashMapService.get(name)
    |> Js.Option.getExn;

  let _buildChunk =
      (
        (top: string, define: string),
        varDeclare: string,
        (funcDeclare: string, funcDefine: string),
        body: string
      ) => {
    top,
    define,
    varDeclare,
    funcDeclare,
    funcDefine,
    body
  };

  let create = () =>
  |};

let _buildInitDataContent = (glslContent: string) => {j|
    WonderCommonlib.MutableHashMapService.{
      chunkMap:
        createEmpty()
        $glslContent
    };
  |j};

let _buildShunkSystemFileContent = (glslContent) =>
  _functionContent ++ _buildInitDataContent(glslContent);

let _writeToShunkSystemFile = (destFilePath, doneFunc, content) => {
  Node.Fs.writeFileSync(destFilePath, content, `utf8);
  [@bs] doneFunc() |> ignore
};

let _convertArrayToList = (array: array(string)) =>
  array |> Js.Array.reduce((list, str) => [str, ...list], []);

let createShunkSystemFile = (glslPathArr: array(string), destFilePath: string, doneFunc) =>
  glslPathArr
  |> Js.Array.map((glslPath) => Glob.sync(glslPath))
  |> ArraySystem.flatten
  |> _convertArrayToList
  |> List.map(
       (actualGlslPath) =>
         Node.Fs.readFileSync(actualGlslPath, `utf8) |> Parse.parseSegment(actualGlslPath)
     )
  |> Parse.parseImport
  |> _buildShunkSystemFileContent
  |> _writeToShunkSystemFile(destFilePath, doneFunc);