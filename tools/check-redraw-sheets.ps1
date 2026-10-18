param([string]$Directory=(Join-Path $PSScriptRoot '../runtime/ai-redraw'))
$ErrorActionPreference='Stop'
if($PSVersionTable.PSEdition -eq 'Core'){
 & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $PSCommandPath -Directory $Directory
 if($LASTEXITCODE -ne 0){throw 'Sheet check failed'}
 return
}
Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.IO;
using System.Drawing;
using System.Collections.Generic;
public static class RedrawSheetCheck {
 public static void Run(string root){
  var rows=new List<string>();rows.Add("asset\tframe\taspect_error_percent\tbottom_delta_source_px\tart_outside_original_mask_percent\toriginal_mask_uncovered_percent\tstatus");
  foreach(int id in new int[]{164,127,136,298}){
   string name="walk_"+id+(id==164?"-v2":"-v1")+".png";
   using(var src=new Bitmap(Path.Combine(root,"walk-source/walk_"+id+".png")))
   using(var art=new Bitmap(Path.Combine(root,"candidates/"+name))){
    double aspect=Math.Abs((double)art.Width/art.Height/((double)src.Width/src.Height)-1)*100;
    for(int row=0;row<4;row++)for(int col=0;col<5;col++){
     int x0=col*art.Width/5,x1=(col+1)*art.Width/5,y0=row*art.Height/4,y1=(row+1)*art.Height/4;
     int pixels=0,outside=0,original=0,missing=0,ab=-1,sb=-1;
     for(int sy=0;sy<48;sy++)for(int sx=0;sx<16;sx++)if(src.GetPixel(col*16+sx,row*48+sy).A>128)sb=Math.Max(sb,sy);
     for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){
      int sx=(x-x0)*16/(x1-x0),sy=(y-y0)*48/(y1-y0);
      bool a=art.GetPixel(x,y).A>128,s=src.GetPixel(col*16+sx,row*48+sy).A>128;
      if(a){pixels++;ab=Math.Max(ab,y-y0);if(!s)outside++;}
      if(s){original++;if(!a)missing++;}
     }
     double delta=ab<0?999:(double)(ab+1)*48/(y1-y0)-(sb+1),extra=100.0*outside/Math.Max(1,pixels),uncovered=100.0*missing/Math.Max(1,original);
     rows.Add(name+"\t"+(row*5+col)+"\t"+aspect.ToString("F3",System.Globalization.CultureInfo.InvariantCulture)+"\t"+delta.ToString("F2",System.Globalization.CultureInfo.InvariantCulture)+"\t"+extra.ToString("F2",System.Globalization.CultureInfo.InvariantCulture)+"\t"+uncovered.ToString("F2",System.Globalization.CultureInfo.InvariantCulture)+"\tmanual-registration-required");
    }
   }
  }
  File.WriteAllLines(Path.Combine(root,"sheet-qa.tsv"),rows);
 }
}
'@
[RedrawSheetCheck]::Run((Resolve-Path -LiteralPath $Directory).Path)
Write-Output 'Checked 80 walking frames; results in sheet-qa.tsv. Metrics do not verify pose or identity.'
