param(
    [string]$GameDirectory=(Join-Path $PSScriptRoot '../runtime/game'),
    [string]$OutputDirectory=(Join-Path $PSScriptRoot '../runtime/ai-redraw/walk-source')
)
$ErrorActionPreference='Stop'
if($PSVersionTable.PSEdition -eq 'Core'){
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $PSCommandPath -GameDirectory $GameDirectory -OutputDirectory $OutputDirectory
    if($LASTEXITCODE -ne 0){throw 'Sprite export failed.'}
    return
}
Add-Type -AssemblyName System.Drawing
$game=(Resolve-Path -LiteralPath $GameDirectory).Path
$output=[IO.Path]::GetFullPath($OutputDirectory)
if($output -eq $game -or $output.StartsWith($game+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Export outside the game directory.'}
New-Item -ItemType Directory -Force -Path $output | Out-Null
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections.Generic;
public static class WalkSpriteExport {
 static int U(byte[] b,int p){if(p<0 || p+2>b.Length)throw new InvalidDataException("Truncated record");return BitConverter.ToUInt16(b,p);}
 public static int Run(string game,string output){
  byte[] ni=File.ReadAllBytes(Path.Combine(game,"NPC.IDX")),nb=File.ReadAllBytes(Path.Combine(game,"NPB.IDX")),
   desc=File.ReadAllBytes(Path.Combine(game,"NPC.DAT")),data=File.ReadAllBytes(Path.Combine(game,"NPB.DAT")),pal=File.ReadAllBytes(Path.Combine(game,"COMMON.PAL"));
  if(ni.Length!=nb.Length || ni.Length%4!=0 || pal.Length!=768)throw new InvalidDataException("Unexpected archive layout");
  var rows=new List<string>();rows.Add("id\twidth\theight\tnormal_frames\tnormal_directions\talternate_frames\talternate_directions\tsheet\tstatus");int total=0;
  for(int id=0;id<ni.Length/4;id++){
   int a=BitConverter.ToInt32(ni,id*4),b=BitConverter.ToInt32(nb,id*4);
   if(a<0 || a+18>desc.Length || b<0 || b+6>data.Length){rows.Add(id+"\t0\t0\t0\t0\t0\t0\t\tunused-or-invalid");continue;}
   int w=U(desc,a),h=U(desc,a+2),nf=U(desc,a+4),af=U(desc,a+6),nd=U(desc,a+8),ad=U(desc,a+10);
   long count=(long)w*h*(nf*nd+af*ad);
   if(w<1 || w>16 || h<1 || h>16 || nf>64 || af>64 || nd>16 || ad>16 || count<1 || count>65535 || a+18+count*2>desc.Length || b+6+count*256>data.Length || U(data,b)!=count || U(data,b+2)!=16 || U(data,b+4)!=16){rows.Add(id+"\t0\t0\t0\t0\t0\t0\t\tunused-or-invalid");continue;}
   string name="walk_"+id.ToString("D3")+".png";
   // Rows: normal directions, then alternate directions. Columns: frame number.
   using(var image=new Bitmap(w*16*Math.Max(nf,af),h*16*(nd+(af>0?ad:0)),PixelFormat.Format32bppArgb)){
    int index=0;
    for(int group=0;group<2;group++)for(int direction=0;direction<(group==0?nd:ad);direction++)for(int frame=0;frame<(group==0?nf:af);frame++)
     for(int ty=0;ty<h;ty++)for(int tx=0;tx<w;tx++){
      int tile=Math.Abs((int)(short)U(desc,a+18+2*index++))&4095;if(tile>=count)throw new InvalidDataException("Tile outside sprite record: "+id);
      for(int y=0;y<16;y++)for(int x=0;x<16;x++){
       int c=data[b+6+tile*256+(x%4)*64+y*4+x/4];
       image.SetPixel(frame*w*16+tx*16+x,((group==0?0:nd)+direction)*h*16+ty*16+y,c==255?Color.Transparent:Color.FromArgb(255,Math.Min(255,pal[c*3]*4),Math.Min(255,pal[c*3+1]*4),Math.Min(255,pal[c*3+2]*4)));
      }
     }
    image.Save(Path.Combine(output,name),ImageFormat.Png);
   }
   rows.Add(id+"\t"+(w*16)+"\t"+(h*16)+"\t"+nf+"\t"+nd+"\t"+af+"\t"+ad+"\t"+name+"\texported");total++;
  }
  File.WriteAllLines(Path.Combine(output,"inventory.tsv"),rows);return total;
 }
}
'@
[WalkSpriteExport]::Run($game,$output)
