using System;
using System.IO;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Collections.Generic;

public static class BattleArtExport {
 static int U(byte[] b,int p){if(p<0 || p+2>b.Length)throw new InvalidDataException("Truncated word");return BitConverter.ToUInt16(b,p);}
 static int Byte(byte[] b,ref int p,int end){if(p>=end)throw new InvalidDataException("Packet exceeds record");return b[p++];}
 static int Word(byte[] b,ref int p,int end){int lo=Byte(b,ref p,end);return lo|(Byte(b,ref p,end)<<8);}
 static void Write(byte[] image,ref int dest,int value){if(dest<0 || dest>=64000)throw new InvalidDataException("Frame exceeds 320x200");image[dest++]=(byte)value;}
 static void Decode(byte[] file,int start,int length,byte[] image){
  int end=checked(start+length),p=start,dest=0;
  if(end>file.Length || Word(file,ref p,end)!=0x42)throw new InvalidDataException("Invalid packet signature");
  int mode=Word(file,ref p,end);
  if(mode==0){for(int i=0;i<64000;i++)Write(image,ref dest,Byte(file,ref p,end));return;}
  if(mode!=1)throw new InvalidDataException("Unsupported compression");
  while(true){
   int command=Byte(file,ref p,end),count;
   if(command==0){count=Byte(file,ref p,end);int value=Byte(file,ref p,end);for(int i=0;i<count;i++)Write(image,ref dest,value);}
   else if(command==128){
    count=Word(file,ref p,end);if(count==0)break;
    if((count&32768)==0)dest+=count;
    else if((count&16384)!=0){count&=16383;int value=Byte(file,ref p,end);for(int i=0;i<count;i++)Write(image,ref dest,value);}
    else{count&=16383;for(int i=0;i<count;i++)Write(image,ref dest,Byte(file,ref p,end));}
   }else if((command&128)!=0)dest+=command&127;
   else for(int i=0;i<command;i++)Write(image,ref dest,Byte(file,ref p,end));
   if(dest>64000)throw new InvalidDataException("Skip exceeds canvas");
  }
 }
 static void Save(string path,int w,int h,int[] pixels){
  using(var bitmap=new Bitmap(w,h,PixelFormat.Format32bppArgb)){
   var bits=bitmap.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb);
   try{for(int y=0;y<h;y++)Marshal.Copy(pixels,y*w,IntPtr.Add(bits.Scan0,y*bits.Stride),w);}finally{bitmap.UnlockBits(bits);}
   bitmap.Save(path,ImageFormat.Png);
  }
 }
 public static string Run(string game,string output){
  var rows=new List<string>();rows.Add("animation\tframes\tcrop_x\tcrop_y\twidth\theight\tstatus\tdetail");int animations=0,frames=0,errors=0;
  string[] files=Directory.GetFiles(Path.Combine(game,"FIGHT"),"*.ANM");Array.Sort(files,StringComparer.OrdinalIgnoreCase);
  foreach(string path in files){string name=Path.GetFileNameWithoutExtension(path);
   try{
    byte[] file=File.ReadAllBytes(path);if(file.Length<2816 || Encoding.ASCII.GetString(file,0,4)!="LPF ")throw new InvalidDataException("Invalid LPF header");
    int pages=U(file,6),limit=U(file,8)-1,frame=0;bool enemy=name.StartsWith("ENEMY",StringComparison.OrdinalIgnoreCase);
    bool actor=enemy || name.StartsWith("FRIEN",StringComparison.OrdinalIgnoreCase);
    if(limit<1 || limit>512 || pages<1 || pages>256)throw new InvalidDataException("Invalid page/frame count");
    int x=actor?(enemy?1:14):0,y=actor?(enemy?1:27):0,w=actor?220:320,h=actor?148:200;byte[] canvas=new byte[64000],pal=new byte[768];int[] colors=new int[256];
    for(int i=0;i<256;i++){
     pal[i*3]=(byte)(file[256+i*4+2]>>2);pal[i*3+1]=(byte)(file[256+i*4+1]>>2);pal[i*3+2]=(byte)(file[256+i*4]>>2);
     colors[i]=i==255?0:Color.FromArgb(255,pal[i*3]*4,pal[i*3+1]*4,pal[i*3+2]*4).ToArgb();
    }
    string folder=Path.Combine(output,name);Directory.CreateDirectory(folder);int columns=4,sw=w*columns,sh=h*((limit+columns-1)/columns);int[] sheet=new int[sw*sh];
    for(int page=0;page<pages && frame<limit;page++){
     int table=1280+page*6,baseFrame=U(file,table),records=U(file,table+2),at=2816+page*65536;
     if(baseFrame!=frame || U(file,at)!=baseFrame || U(file,at+2)!=records)throw new InvalidDataException("Page sequence mismatch");
     int packet=at+8+records*2;
     for(int record=0;record<records && frame<limit;record++,frame++){
      int length=U(file,at+8+record*2);if(length>0)Decode(file,packet,length,canvas);packet+=length;
      int[] pixels=new int[w*h];byte[] reference=new byte[768+w*h];Array.Copy(pal,reference,768);
      for(int cy=0;cy<h;cy++)for(int cx=0;cx<w;cx++){
       int c=canvas[(y+cy)*320+x+cx];pixels[cy*w+cx]=colors[c];reference[768+cy*w+cx]=(byte)c;
       sheet[((frame/columns)*h+cy)*sw+(frame%columns)*w+cx]=colors[c];
      }
      Save(Path.Combine(folder,"frame_"+frame.ToString("D3")+".png"),w,h,pixels);
      File.WriteAllBytes(Path.Combine(folder,"frame_"+frame.ToString("D3")+".idx"),reference);
     }
    }
    if(frame!=limit)throw new InvalidDataException("Incomplete frames");
    Save(Path.Combine(output,name+"-sheet.png"),sw,sh,sheet);
    rows.Add(name+"\t"+frame+"\t"+x+"\t"+y+"\t"+w+"\t"+h+"\texported\tLPF delta frames; final loop record excluded as in engine");animations++;frames+=frame;
   }catch(Exception e){rows.Add(name+"\t0\t0\t0\t0\t0\terror\t"+e.Message.Replace('\t',' '));errors++;}
  }
  File.WriteAllLines(Path.Combine(output,"inventory.tsv"),rows,new UTF8Encoding(false));return "animations="+animations+" frames="+frames+" errors="+errors;
 }
}
