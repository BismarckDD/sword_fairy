(async function(scene,jobs,prompt) {
 if(!/^[A-Z0-9]+$/.test(scene)||jobs.some(j=>!j.startsWith(scene+"_")||!/^[A-Z0-9_]+$/.test(j)))throw Error("Invalid job");
 const stamp=Date.now(), folder="runtime/game/hd/maps/context-tiles/"+scene+"-320";
 const promptFile=folder+"/prompts-batch-"+stamp+".txt";
 await tools.apply_patch("*** Begin Patch\n*** Add File: "+promptFile+"\n+Built-in imagegen. Original crops are edit targets. Jobs: "+jobs.join(", ")+"\n+\n+"+prompt+"\n+Append Scene and job ID.\n*** End Patch");
 let queue=Promise.resolve();
 const results=await Promise.allSettled(jobs.map(async job=>{
  const r=await tools.image_gen__imagegen({referenced_image_paths:["D:\\workplace\\sword_fairy\\runtime\\game\\hd\\maps\\context-tiles\\"+scene+"-320\\source\\"+job+".png"],transparent_background:false,prompt:prompt+" Scene "+job});
  store(job,r);generatedImage(r);
  const file=r.output_hint.match(/as (C:\\[^\r\n]+\.png) by default/)[1];
  const task=queue.then(async()=>{
   const cmd="& 'C:\\Users\\mayan\\.cache\\codex-runtimes\\codex-primary-runtime\\dependencies\\python\\python.exe' 'C:\\Users\\mayan\\.codex\\skills\\game-map-redraw\\scripts\\map_redraw.py' register --project-root D:\\workplace\\sword_fairy --scene "+scene+" --job "+job+" --source '"+file.replaceAll("'","''")+"'";
   let v=await tools.exec_command({cmd,max_output_tokens:300});text(v);
   while(v.session_id){v=await tools.write_stdin({session_id:v.session_id,chars:"",yield_time_ms:1000,max_output_tokens:300});text(v);}
   if(v.exit_code!==0)throw Error("Registration failed "+job);
  });queue=task;await task;
 }));
 const failures=results.flatMap((r,i)=>r.status==="rejected"?[{job:jobs[i],error:String(r.reason)}]:[]);
 if(failures.length){text(failures);await tools.apply_patch("*** Begin Patch\n*** Add File: "+folder+"/generation-failures-"+stamp+".json\n+"+JSON.stringify(failures,null,2).split("\n").join("\n+")+"\n*** End Patch");}
})
