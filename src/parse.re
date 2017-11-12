open Type;

/* @define
   #import "common_vertex"


   @end

   @funcDefine
   #import "common_vertex"
   @end */
let _getFileName = (path: string) => Node.Path.basenameExt(path, PathExtend.extname(path));

let _buildGlslContent = (name: string, (top, define, varDeclare, funcDeclare, funcDefine, body)) => {j|
|> set("$name", _buildChunk("$top","$define","$varDeclare","$funcDeclare","$funcDefine","$body")
|j};

let _checkCircleImport = (segmentContent: string) => {};

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
let rec _getAllImportContent = (segmentName: string, segmentContent: string, map) => {
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
  let importFlagRe = [%re {|/#import\s+"(.+)"/g|}];
  let recordArr = [||];
  let startIndex = ref(0);
  let break = ref(false);
  while (! break^) {
    switch (importFlagRe |> Js.Re.exec(segmentContent)) {
    | None => break := true
    | Some(result) =>
      /* let endIndex = Js.Re.lastIndex(result); */
      recordArr
      |> Js.Array.push
           /* Js.Re.index(result), Js.Re.lastIndex(result),  */
           ((
             startIndex,
             Js.Re.index(result),
             /* endIndex, */
             Js.Nullable.bind(
               Js.Re.captures(result)[1],
               [@bs]
               (
                 (importFileName) =>
                   switch (Js.Dict.get(map, importFileName)) {
                   | None => failwith({j|import glsl file:$importFileName should exist|j})
                   | Some(segmentMap) =>
                     switch (Js.Dict.get(segmentMap, segmentName)) {
                     | None =>
                       failwith({j|segment:$segmentName should exist in $importFileName.glsl|j})
                     | Some(importContent) =>
                       if (importFlagRe |> Js.Re.test(importContent)) {
                         _getAllImportContent(segmentName, importContent, map)
                       } else {
                         importContent
                       }
                     }
                   }
               )
             )
           ));
      startIndex := Js.Re.lastIndex(importFlagRe)
    }
  };
  recordArr
  |> Js.Array.reduce(
       (content, (startIndex, endIndex, importSegmentContent)) =>
         content
         ++ (segmentContent |> Js.String.slice(~from=startIndex, ~to_=endIndex))
         ++ importSegmentContent,
       ""
     )
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
  /* todo check: shouldn't has circle import! */
  /* content; */
  /* (top, define, varDeclare, funcDeclare, funcDefine, body) */
  let map = _convertListToMap(list);
  list
  |> List.fold_left
       /* (content, (fileName, top, define, varDeclare, funcDeclare, funcDefine, body)) => { */
       (
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
           /* _getAllImportContent("top", segmentContent: string, map) */
           /* content ++ _getAllImportContent(topKey, topContent, map)
               ++ _getAllImportContent(defineKey, defineContent, map)
              ++ _getAllImportContent(varDeclareKey, varDeclareContent, map)
              ++ _getAllImportContent(funcDeclareKey, funcDeclareContent, map)
              ++ _getAllImportContent(funcDefineKey, funcDefineContent, map)
              ++ _getAllImportContent(bodyKey, bodyContent, map) */
           content
           ++ _buildGlslContent(
                fileName,
                (
                  _getAllImportContent(topKey, topContent, map),
                  _getAllImportContent(defineKey, defineContent, map),
                  _getAllImportContent(varDeclareKey, varDeclareContent, map),
                  _getAllImportContent(funcDeclareKey, funcDeclareContent, map),
                  _getAllImportContent(funcDefineKey, funcDefineContent, map),
                  _getAllImportContent(bodyKey, bodyContent, map)
                )
              ),
         ""
       )
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
         | startIndex =>
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