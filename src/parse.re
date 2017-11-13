/* @define
   #import "common_vertex"


   @end

   @funcDefine
   #import "common_vertex"
   @end */
let _getFileName = (path: string) => Node.Path.basenameExt(path, PathExtend.extname(path));

/* todo check fileName should be name, not path! */
/* todo check fileName should not has .glsl! */
let _buildGlslContent = (name: string, (top, define, varDeclare, funcDeclare, funcDefine, body)) =>
  switch (PathExtend.extname(name)) {
  | "" =>
    switch (name |> Js.String.startsWith("./"), name |> Js.String.startsWith("../")) {
    | (false, false) => {j|
|> set("$name", _buildChunk("$top","$define","$varDeclare","$funcDeclare","$funcDefine","$body"))
|j}
    | (_, _) => failwith({j|should import fileName, not filePath|j})
    }
  | extname => failwith({j|should import fileName without $extname|j})
  };

/* let _checkCircleImport = (segmentContent: string) => {}; */
/* let _getEndImportContent = (fileName, segmentName:string, segmentContent: string, map):string => {
     let importFlagRe = [%re {|/#import\s+"(.+)"/g|}];
     let recordArr = [];
     let startIndex = ref(0);
     let break = ref(false);
   while(!( break^ )){
              switch (importFlagRe |> Js.Re.exec(segmentContent)) {
              | None => break := true;
              | Some(result) =>
              /* let endIndex = Js.Re.lastIndex(result); */
              recordArr |> Js.Array.push(

              (
                  /* Js.Re.index(result), Js.Re.lastIndex(result),  */
                  startIndex,
   Js.Re.index(result),
                  /* endIndex, */

              Js.Nullable.iter(Js.Re.captures(result)[1], (importFileName) => {

   switch(map |> Js.Dict.get(importFileName)){
   | None => failwith({j|import glsl file:$importFileName should exist|j})
   | Some(map) =>
   switch(map |> Js.Dict.get(segmentName)){
   | None => failwith({j|segment:$segmentName should exist in $importFileName.glsl|j})
   | Some(importContent) =>
   importContent

   }
   }



              })
              )
              );

              startIndex := Js.Re.lastIndex(result);

              }
   };

   recordArr |> Js.Array.reduce((content, (startIndex, endIndex, importSegmentContent)) => {
       content ++
       segmentContent |> Js.String.slice(~from=startIndex, ~to_=endIndex)
       ++ importSegmentContent
   }, "");
   }; */
let _getAllImportContent = (fileName: string, segmentName: string, segmentContent: string, map) => {
  /* let importFlagRe = [%re {|/#import\s+"(.+)"/g|}];
       let break = ref(false);
     while(!( break^ )){
                switch (importFlagRe |> Js.Re.exec(segmentContent)) {
                | None => break := true;
                | Some(result) =>

                Js.Re.captures(result)[1]
     switch(map |> Js.Dict.get(importFileName)){
     | None => failwith({j|import glsl file:$importFileName should exist|j})
     | Some(map) =>
     switch(map |> Js.Dict.get(segmentName)){
     | None => failwith({j|segment:$segmentName should exist in $importFileName.glsl|j})
     | Some(importContent) =>


     }
     }
                }
     } */
  let _createImportFlagRe = () => [%re {|/#import\s+"(.+)"/g|}];
  let rec _get = (fileNameList: list(string), segmentName: string, segmentContent: string, map) => {
    let importFlagRe = _createImportFlagRe();
    let recordArr = [||];
    let startIndex = ref(0);
    let break = ref(false);
    /* DebugUtils.log((fileNameList, segmentName)) |> ignore; */
    while (! break^) {
      switch (importFlagRe |> Js.Re.exec(segmentContent)) {
      | None =>
        recordArr |> Js.Array.push((startIndex^, Js.String.length(segmentContent), ""));
        /* DebugUtils.log((Js.String.length(segmentContent))) |> ignore; */
        /* DebugUtils.log((startIndex^, Js.String.length(segmentContent))) |> ignore; */
        break := true
      | Some(result) =>
        recordArr
        |> Js.Array.push
             /* Js.Re.index(result), Js.Re.lastIndex(result),  */
             ((
               startIndex^,
               Js.Re.index(result),
               /* endIndex, */
               Js.Nullable.bind(
                 Js.Re.captures(result)[1],
                 [@bs]
                 (
                   (importFileName) =>
                     switch (PathExtend.extname(importFileName)) {
                     | "" =>
                       switch (
                         importFileName |> Js.String.startsWith("./"),
                         importFileName |> Js.String.startsWith("../")
                       ) {
                       | (false, false) =>
                         switch (Js.Dict.get(map, importFileName)) {
                         | None =>
                           /* DebugUtils.log(map) |> ignore;  */
                           failwith({j|import glsl file:$importFileName should exist|j})
                         | Some(segmentMap) =>
                           let fileName = Js.Dict.unsafeGet(segmentMap, "fileName");
                           /* DebugUtils.log(fileName) |> ignore; */
                           let newFileNameList = [fileName, ...fileNameList];
                           if (List.mem(fileName, fileNameList)) {
                             /* DebugUtils.log("false") |> ignore; */
                             let msg =
                               newFileNameList
                               |> List.rev
                               |> List.fold_left((str, fileName) => str ++ fileName ++ "=>", "")
                               |> Js.String.slice(~from=0, ~to_=(-2));
                             failwith(
                               {j|not allow circular reference(the reference path is $msg)|j}
                             )
                           } else {
                             switch (Js.Dict.get(segmentMap, segmentName)) {
                             | None =>
                               failwith(
                                 {j|segment:$segmentName should exist in $importFileName.glsl|j}
                               )
                             | Some(importContent) =>
                               /* DebugUtils.log(importContent) |> ignore; */
                               if (_createImportFlagRe() |> Js.Re.test(importContent)) {
                                 _get(newFileNameList, segmentName, importContent, map)
                               } else {
                                 importContent
                               }
                             }
                           }
                         }
                       | (_, _) => failwith({j|should import fileName, not filePath|j})
                       }
                     | extname => failwith({j|should import fileName without $extname|j})
                     }
                 )
               )
             ))
        |> ignore;
        startIndex := Js.Re.lastIndex(importFlagRe)
      }
    };
    /* if (Js.Array.length(recordArr) > 0) { */
    recordArr
    |> Js.Array.reduce(
         (content, (startIndex, endIndex, importSegmentContent)) =>
           content
           ++ (segmentContent |> Js.String.slice(~from=startIndex, ~to_=endIndex))
           ++ importSegmentContent,
         ""
       )
    /* } else {
         segmentContent
       } */
  };
  _get([fileName], segmentName, segmentContent, map)
};

let _convertListToMap = (list) =>
  /* Js.Dict.fromList */
  /* (fileName, top, define, varDeclare, funcDeclare, funcDefine, body) */
  list
  |> List.fold_left
       /* (map, (fileName, top, define, varDeclare, funcDeclare, funcDefine, body)) => { */
       (
         (
           map,
           (
             fileName,
             (topKey, topContent),
             (defineKey, defineContent),
             (varDeclareKey, varDeclareContent),
             (funcDeclareKey, funcDeclareContent),
             (funcDefineKey, funcDefineContent),
             (bodyKey, bodyContent)
           )
         ) => {
           let segmentMap = Js.Dict.empty();
           Js.Dict.set(segmentMap, "fileName", fileName);
           Js.Dict.set(segmentMap, topKey, topContent);
           Js.Dict.set(segmentMap, defineKey, defineContent);
           Js.Dict.set(segmentMap, varDeclareKey, varDeclareContent);
           Js.Dict.set(segmentMap, funcDeclareKey, funcDeclareContent);
           Js.Dict.set(segmentMap, funcDefineKey, funcDefineContent);
           Js.Dict.set(segmentMap, bodyKey, bodyContent);
           Js.Dict.set(map, fileName, segmentMap);
           map
         },
         Js.Dict.empty()
       );

let parseImport = (list) => {
  let map = _convertListToMap(list);
  list
  |> List.fold_left(
       (
         content,
         (
           fileName,
           (topKey, topContent),
           (defineKey, defineContent),
           (varDeclareKey, varDeclareContent),
           (funcDeclareKey, funcDeclareContent),
           (funcDefineKey, funcDefineContent),
           (bodyKey, bodyContent)
         )
       ) =>
         content
         ++ _buildGlslContent(
              fileName,
              (
                _getAllImportContent(fileName, topKey, topContent, map),
                _getAllImportContent(fileName, defineKey, defineContent, map),
                _getAllImportContent(fileName, varDeclareKey, varDeclareContent, map),
                _getAllImportContent(fileName, funcDeclareKey, funcDeclareContent, map),
                _getAllImportContent(fileName, funcDefineKey, funcDefineContent, map),
                _getAllImportContent(fileName, bodyKey, bodyContent, map)
              )
            ),
       /* DebugUtils.log((topKey, topContent)) |> ignore; */
       ""
     )
  /* _getAllImportContent("top", segmentContent: string, map) */
  /* content ++ _getAllImportContent(topKey, topContent, map)
      ++ _getAllImportContent(defineKey, defineContent, map)
     ++ _getAllImportContent(varDeclareKey, varDeclareContent, map)
     ++ _getAllImportContent(funcDeclareKey, funcDeclareContent, map)
     ++ _getAllImportContent(funcDefineKey, funcDefineContent, map)
     ++ _getAllImportContent(bodyKey, bodyContent, map) */
};

let parseSegment = (actualGlslPath: string, content: string) => {
  /* let segmentFlagList = [("top", "@top" ), ( "define", "@define" ), ("varDeclare", "@varDeclare" ), ("funcDeclare", "@funcDeclare" ), ("funcDefine", "@funcDefine" ), ("body", "@body" )]; */
  let segmentFlagList = ["@top", "@define", "@varDeclare", "@funcDeclare", "@funcDefine", "@body"];
  let endFlagRe = [%re {|/@end/g|}];
  segmentFlagList
  |> List.fold_left(
       (list, flag) => [
         switch (content |> Js.String.indexOf(flag)) {
         | (-1) => ""
         | index =>
           let startIndex = index + Js.String.length(flag);
           switch (endFlagRe |> Js.Re.exec(content)) {
           | None => failwith("@end should match to segement flag")
           | Some(result) =>
             content
             |> Js.String.slice(~from=startIndex, ~to_=Js.Re.index(result))
             |> Js.String.trim
           }
         },
         ...list
       ],
       []
     )
  |> List.rev
  |> (
    (list) =>
      List.(
        _getFileName(actualGlslPath),
        ("top", nth(list, 0)),
        ("define", nth(list, 1)),
        ("varDeclare", nth(list, 2)),
        ("funcDeclare", nth(list, 3)),
        ("funcDefine", nth(list, 4)),
        ("body", nth(list, 5))
      )
  )
};
/* let parse = (actualGlslPath: string, content: string) =>
   content |> _parseSegment |> _parseImport |> _buildGlslContent(_getFileName(actualGlslPath)); */