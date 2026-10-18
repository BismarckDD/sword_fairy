using System;
using System.IO;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
// Deterministic size/format adaptation of existing user-provided artwork.
public static class JxHdImport {
    public static bool Empty(string reference) {
        byte[] b=File.ReadAllBytes(reference);
        for(int i=768;i<b.Length;i++)if(b[i]!=255)return false;
        return true;
    }
    static Rectangle OriginalBounds(byte[] reference,int w,int h) {
        int left=w,top=h,right=-1,bottom=-1;
        for(int y=0;y<h;y++)for(int x=0;x<w;x++)if(reference[768+y*w+x]!=255) {
            left=Math.Min(left,x);top=Math.Min(top,y);right=Math.Max(right,x);bottom=Math.Max(bottom,y);
        }
        return right<0?Rectangle.Empty:new Rectangle(left*4,top*4,(right-left+1)*4,(bottom-top+1)*4);
    }
    static Rectangle ForegroundBounds(Bitmap image) {
        Color bg=image.GetPixel(0,0);int left=image.Width,top=image.Height,right=-1,bottom=-1;
        for(int y=0;y<image.Height;y+=2)for(int x=0;x<image.Width;x+=2) {
            Color c=image.GetPixel(x,y);
            int delta=Math.Abs(c.R-bg.R)+Math.Abs(c.G-bg.G)+Math.Abs(c.B-bg.B);
            if(delta>65) {left=Math.Min(left,x);top=Math.Min(top,y);right=Math.Max(right,x);bottom=Math.Max(bottom,y);}
        }
        if(right<0)return new Rectangle(0,0,image.Width,image.Height);
        left=Math.Max(0,left-2);top=Math.Max(0,top-2);right=Math.Min(image.Width-1,right+2);bottom=Math.Min(image.Height-1,bottom+2);
        return new Rectangle(left,top,right-left+1,bottom-top+1);
    }
    public static string Convert(string source,string reference,string bmp,string png,int width,int height,bool alignSprite) {
        byte[] original=File.ReadAllBytes(reference);
        if(original.Length!=768+width*height)throw new InvalidDataException("Reference dimensions do not match");
        int w=width*4,h=height*4;
        using(Bitmap input=new Bitmap(source))using(Bitmap output=new Bitmap(w,h,PixelFormat.Format32bppArgb)) {
            Rectangle src=new Rectangle(0,0,input.Width,input.Height),dest=new Rectangle(0,0,w,h);
            if(alignSprite) {src=ForegroundBounds(input);dest=OriginalBounds(original,width,height);}
            if(dest.IsEmpty)throw new InvalidDataException("Empty original sprite");
            using(Graphics g=Graphics.FromImage(output))using(ImageAttributes attrs=new ImageAttributes()) {
                g.Clear(Color.FromArgb(255,original[765]*4,original[766]*4,original[767]*4));
                g.CompositingMode=CompositingMode.SourceCopy;g.CompositingQuality=CompositingQuality.HighQuality;
                g.InterpolationMode=InterpolationMode.HighQualityBicubic;g.PixelOffsetMode=PixelOffsetMode.HighQuality;
                attrs.SetWrapMode(WrapMode.TileFlipXY);g.DrawImage(input,dest,src.X,src.Y,src.Width,src.Height,GraphicsUnit.Pixel,attrs);
            }
            byte[] pixels=new byte[w*h*4];Rectangle full=new Rectangle(0,0,w,h);
            BitmapData data=output.LockBits(full,ImageLockMode.ReadWrite,PixelFormat.Format32bppArgb);
            try {
                if(data.Stride!=w*4)throw new InvalidDataException("Unsupported bitmap stride");
                Marshal.Copy(data.Scan0,pixels,0,pixels.Length);
                // The engine also retains this original mask, protecting scene backgrounds.
                for(int y=0;y<h;y++)for(int x=0;x<w;x++)pixels[(y*w+x)*4+3]=(byte)(original[768+(y/4)*width+x/4]==255?0:255);
                Marshal.Copy(pixels,0,data.Scan0,pixels.Length);
            }finally{output.UnlockBits(data);}
            using(BinaryWriter f=new BinaryWriter(File.Create(bmp))) {
                f.Write((byte)'B');f.Write((byte)'M');f.Write(54+pixels.Length);f.Write(0);f.Write(54);
                f.Write(40);f.Write(w);f.Write(-h);f.Write((short)1);f.Write((short)32);f.Write(0);f.Write(pixels.Length);
                for(int i=0;i<4;i++)f.Write(0);f.Write(pixels);
            }
            output.Save(png,ImageFormat.Png);
            return String.Format("source={0}x{1}; target={2}x{3}; crop={4},{5},{6},{7}; destination={8},{9},{10},{11}",
                input.Width,input.Height,w,h,src.X,src.Y,src.Width,src.Height,dest.X,dest.Y,dest.Width,dest.Height);
        }
    }
}
