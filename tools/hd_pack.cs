using System;
using System.IO;
using System.Text;
using System.Collections.Generic;
// Deterministic indexed-pixel asset conversion; no palette recoloring or AI redraw.
public static class JxHdPack {
    static byte[] Scale2(byte[] s,int w,int h) {
        byte[] d=new byte[w*h*4];
        for(int y=0;y<h;y++)for(int x=0;x<w;x++) {
            byte e=s[y*w+x],b=s[(y>0?y-1:y)*w+x],g=s[(y+1<h?y+1:y)*w+x],
                l=s[y*w+(x>0?x-1:x)],r=s[y*w+(x+1<w?x+1:x)];
            int p=y*2*w*2+x*2;
            d[p]=(l==b && b!=r && l!=g)?l:e;
            d[p+1]=(b==r && b!=l && r!=g)?r:e;
            d[p+w*2]=(l==g && l!=b && g!=r)?l:e;
            d[p+w*2+1]=(g==r && l!=g && b!=r)?r:e;
        }
        return d;
    }
    static void Bmp(string path,byte[] indices,byte[] palette,int w,int h) {
        byte[] pixels=new byte[w*h*16];
        for(int y=0;y<h*4;y++)for(int x=0;x<w*4;x++)pixels[y*w*4+x]=indices[(y/4)*w+x/4];
        w*=4;h*=4;
        using(BinaryWriter f=new BinaryWriter(File.Create(path))) {
            f.Write((byte)'B');f.Write((byte)'M');f.Write(54+w*h*4);f.Write(0);f.Write(54);
            f.Write(40);f.Write(w);f.Write(-h);f.Write((short)1);f.Write((short)32);
            f.Write(0);f.Write(w*h*4);f.Write(0);f.Write(0);f.Write(0);f.Write(0);
            for(int i=0;i<pixels.Length;i++) {
                int c=pixels[i]*3;
                f.Write((byte)Math.Min(255,palette[c+2]*4));f.Write((byte)Math.Min(255,palette[c+1]*4));
                f.Write((byte)Math.Min(255,palette[c]*4));f.Write((byte)255);
            }
        }
    }
    static void Entry(List<string> lines,HashSet<string> trueColors,string output,string key,byte[] src,int start,int w,int h,byte[] palette) {
        if(w<1 || h<1 || w>320 || h>200 || w*h>64000)return;
        byte[] indices=new byte[w*h];Buffer.BlockCopy(src,start,indices,0,indices.Length);
        string image="images/"+key+".bmp",reference="references/"+key+".idx";
        // Preserve hand-edited replacement images on repeat runs.
        bool external=File.Exists(Path.Combine(output,image)) && trueColors.Contains(key);
        if(!File.Exists(Path.Combine(output,image)))Bmp(Path.Combine(output,image),indices,palette,w,h);
        using(FileStream f=File.Create(Path.Combine(output,reference))) {f.Write(palette,0,768);f.Write(indices,0,indices.Length);}
        lines.Add(key+"\t"+w+"\t"+h+"\t"+reference+"\t"+image+(external?"\ttruecolor":""));
    }
    public static int Build(string game,string output) {
        HashSet<string> trueColors=new HashSet<string>(StringComparer.Ordinal);
        string prior=Path.Combine(output,"manifest.tsv");
        if(File.Exists(prior))foreach(string line in File.ReadAllLines(prior)) {
            string[] fields=line.Split('\t');if(fields.Length==6 && fields[5]=="truecolor")trueColors.Add(fields[0]);
        }
        Directory.CreateDirectory(Path.Combine(output,"images"));Directory.CreateDirectory(Path.Combine(output,"references"));
        List<string> lines=new List<string>();lines.Add("# JXHD1\tNearest4x\t320x200 -> 1280x800");
        List<string> files=new List<string>(Directory.GetFiles(game,"*.IMG"));
        if(Directory.Exists(Path.Combine(game,"FIGHT")))files.AddRange(Directory.GetFiles(Path.Combine(game,"FIGHT"),"*.IMG"));
        files.Sort(StringComparer.OrdinalIgnoreCase);
        foreach(string file in files) {
            byte[] b=File.ReadAllBytes(file);
            if(b.Length<784 || Encoding.ASCII.GetString(b,0,4)!="IMGV")continue;
            int w=BitConverter.ToUInt16(b,6)+1,h=BitConverter.ToUInt16(b,8)+1;
            if(w>320 || h>200 || w*h<1 || (b.Length-784)%(w*h)!=0)continue;
            byte[] palette=new byte[768];Buffer.BlockCopy(b,b.Length-768,palette,0,768);
            string name=file.Substring(game.Length+1).Replace('\\','_').Replace('/','_').Replace('.','_').ToUpperInvariant();
            for(int frame=0;frame<(b.Length-784)/(w*h);frame++)Entry(lines,trueColors,output,name+"_"+frame.ToString("D3"),b,16+frame*w*h,w,h,palette);
        }
        string face=Path.Combine(game,"FACE.INF"),common=Path.Combine(game,"COMMON.PAL");
        if(File.Exists(face) && File.Exists(common)) {
            byte[] b=File.ReadAllBytes(face),palette=File.ReadAllBytes(common);
            if(b.Length%4200==0 && palette.Length==768)
                for(int frame=0;frame<b.Length/4200;frame++)Entry(lines,trueColors,output,"FACE_INF_"+frame.ToString("D3"),b,frame*4200,60,70,palette);
        }
        File.WriteAllLines(Path.Combine(output,"manifest.tsv"),lines,new UTF8Encoding(false));
        return lines.Count-1;
    }
    public static string RestoreOriginalPixels(string hd,string backup) {
        int updated=0,exact=0,custom=0;List<string> report=new List<string>();Directory.CreateDirectory(backup);
        foreach(string line in File.ReadAllLines(Path.Combine(hd,"manifest.tsv"))) {
            if(line.StartsWith("#") || line.Length==0)continue;string[] fields=line.Split('\t');
            if(fields.Length!=5)continue; // Preserve every imported truecolor image.
            if(fields[3].Contains("..") || fields[4].Contains("..") || !fields[3].StartsWith("references/") || !fields[4].StartsWith("images/"))throw new InvalidDataException("Unsafe asset path");
            int w=Int32.Parse(fields[1]),h=Int32.Parse(fields[2]);
            if(w<1 || w>320 || h<1 || h>200)throw new InvalidDataException("Invalid asset dimensions");
            string path=Path.Combine(hd,fields[4]);byte[] reference=File.ReadAllBytes(Path.Combine(hd,fields[3])),bmp=File.ReadAllBytes(path);
            if(reference.Length!=768+w*h || bmp.Length!=54+w*h*64 || BitConverter.ToInt32(bmp,10)!=54 || BitConverter.ToInt32(bmp,18)!=w*4 || BitConverter.ToInt32(bmp,22)!=-h*4){custom++;continue;}
            byte[] indices=new byte[w*h];Buffer.BlockCopy(reference,768,indices,0,indices.Length);
            byte[] smooth=Scale2(Scale2(indices,w,h),w*2,h*2);bool legacy=true,nearest=true;
            for(int y=0;y<h*4;y++)for(int x=0;x<w*4;x++) {
                int i=y*w*4+x,plain=indices[(y/4)*w+x/4],old=smooth[i];
                for(int ch=0;ch<3;ch++) {
                    byte value=bmp[54+i*4+ch];
                    if(value!=Math.Min(255,reference[plain*3+2-ch]*4))nearest=false;
                    if(value!=Math.Min(255,reference[old*3+2-ch]*4))legacy=false;
                }
            }
            if(nearest){exact++;continue;}
            if(!legacy){custom++;report.Add("custom_preserved\t"+fields[0]);continue;}
            File.Copy(path,Path.Combine(backup,Path.GetFileName(path)),false);
            for(int y=0;y<h*4;y++)for(int x=0;x<w*4;x++) {
                int i=y*w*4+x,index=indices[(y/4)*w+x/4];
                for(int ch=0;ch<3;ch++)bmp[54+i*4+ch]=(byte)Math.Min(255,reference[index*3+2-ch]*4);
            }
            File.WriteAllBytes(path,bmp);updated++;report.Add("restored\t"+fields[0]);
        }
        string summary="restored="+updated+"; already_exact="+exact+"; custom_preserved="+custom;
        report.Insert(0,summary);File.WriteAllLines(Path.Combine(backup,"report.txt"),report,new UTF8Encoding(false));return summary;
    }
}
