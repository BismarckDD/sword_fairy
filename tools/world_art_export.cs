using System;
using System.IO;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Collections.Generic;

// Offline export only. Never writes to original game archives or the active HD pack.
public static class WorldArtExport {
 static int U(byte[] b,int p){if(p<0 || p+2>b.Length)throw new InvalidDataException("Truncated word");return BitConverter.ToUInt16(b,p);}
 static string Name(byte[] b,int p){return Encoding.ASCII.GetString(b,p,12).TrimEnd('\0',' ');}
 static Dictionary<string,int> Index(byte[] b,int length){
  if(b.Length%16!=0)throw new InvalidDataException("Invalid archive index");
  var result=new Dictionary<string,int>(StringComparer.OrdinalIgnoreCase);
  for(int i=0;i<b.Length;i+=16){int at=BitConverter.ToInt32(b,i+12);if(at<0 || at>=length)throw new InvalidDataException("Invalid archive offset");result.Add(Name(b,i),at);}
  return result;
 }
 static void Save(string path,int w,int h,int[] pixels){
  using(var bitmap=new Bitmap(w,h,PixelFormat.Format32bppArgb)){
   var bits=bitmap.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb);
   try{for(int y=0;y<h;y++)Marshal.Copy(pixels,y*w,IntPtr.Add(bits.Scan0,y*bits.Stride),w);}finally{bitmap.UnlockBits(bits);}
   bitmap.Save(path,ImageFormat.Png);
  }
 }
 public static string Run(string game,string output){
  byte[] maps=File.ReadAllBytes(Path.Combine(game,"MAP.DAT")),blocks=File.ReadAllBytes(Path.Combine(game,"BLK.DAT")),common=File.ReadAllBytes(Path.Combine(game,"COMMON.PAL"));
  if(common.Length!=768)throw new InvalidDataException("Unexpected common palette");
  var mi=Index(File.ReadAllBytes(Path.Combine(game,"MAP.IDX")),maps.Length);
  var bi=Index(File.ReadAllBytes(Path.Combine(game,"BLK.IDX")),blocks.Length);
  var rows=new List<string>();rows.Add("map\twidth\theight\tblock\tpalette\tstatus\tdetail");int total=0,failed=0;
  foreach(var entry in mi){
   if(entry.Key.EndsWith(".PAL",StringComparison.OrdinalIgnoreCase))continue;
   string name=entry.Key;if(name.IndexOfAny(Path.GetInvalidFileNameChars())>=0)throw new InvalidDataException("Unsafe member name");
   try{
    int m=entry.Value,w=U(maps,m),h=U(maps,m+2);string blockName=Name(maps,m+4),paletteName=Name(maps,m+16);
    if(w<1 || h<1 || w>256 || h>256 || m+28+4L*w*h>maps.Length)throw new InvalidDataException("Invalid map dimensions");
    int b=bi[blockName],p=mi[paletteName],count=U(blocks,b);
    if(U(blocks,b+2)!=16 || U(blocks,b+4)!=16 || b+6L+count*256>blocks.Length || p+787>maps.Length)throw new InvalidDataException("Invalid blocks or palette");
    byte[] pal=(byte[])common.Clone();Array.Copy(maps,p+22+480,pal,480,285);
    int[] colors=new int[256];for(int i=0;i<256;i++)colors[i]=i==255?0:Color.FromArgb(255,Math.Min(255,pal[i*3]*4),Math.Min(255,pal[i*3+1]*4),Math.Min(255,pal[i*3+2]*4)).ToArgb();
    int width=w*16,height=h*16;int[] scene=new int[width*height];
    for(int layer=0;layer<2;layer++)for(int cy=0;cy<h;cy++)for(int cx=0;cx<w;cx++){
     int tile=Math.Abs((int)(short)U(maps,m+28+2*(layer*w*h+cy*w+cx)))&4095;
     if(layer==1 && tile==1)continue;
     if(tile>=count)throw new InvalidDataException("Map references absent block");
     for(int y=0;y<16;y++)for(int x=0;x<16;x++){
      int c=blocks[b+6+tile*256+(x%4)*64+y*4+x/4];if(c!=255)scene[(cy*16+y)*width+cx*16+x]=colors[c];
     }
    }
    Save(Path.Combine(output,name+".png"),width,height,scene);
    // Store a per-map tile atlas: the same blocks may use different scene palettes.
    int aw=256,ah=((count+15)/16)*16;int[] atlas=new int[aw*ah];
    for(int t=0;t<count;t++)for(int y=0;y<16;y++)for(int x=0;x<16;x++)atlas[((t/16)*16+y)*aw+(t%16)*16+x]=colors[blocks[b+6+t*256+(x%4)*64+y*4+x/4]];
    Save(Path.Combine(output,name+"-tiles.png"),aw,ah,atlas);
    rows.Add(name+"\t"+width+"\t"+height+"\t"+blockName+"\t"+paletteName+"\texported\tstatic layers; no actors or scripted objects");total++;
   }catch(Exception e){rows.Add(name+"\t0\t0\t\t\terror\t"+e.Message.Replace('\t',' '));failed++;}
  }
  File.WriteAllLines(Path.Combine(output,"inventory.tsv"),rows,new UTF8Encoding(false));
  return "maps="+total+" errors="+failed;
 }
}
