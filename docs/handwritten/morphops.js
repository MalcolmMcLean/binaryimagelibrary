function dilate(binary, width, height, sel, swidth, sheight)
{
  var x, y;
  var sx, sy;
  var csx, csy;
  var ix, iy;
  var bit;
  var i;
  var buff = [];

  
  width = binary.width;
  height = binary.height;
  csx = Math.floor(swidth/2);
  csy = Math.floor(sheight/2);

  buff.fill(0, 0, width*height);
  
   for(y=0;y<height;y++)
   {
      for(x=0;x<width;x++)
      {
        for(sy=0;sy<sheight;sy++)
        {
           for(sx =0; sx < swidth; sx++)
	    {
		  ix = x + sx - csx;
	        iy = y + sy - csy;
             if(ix < 0 || ix >= width || iy < 0 || iy >= height)
			bit = 0;
		else
		       bit = binary.data[(iy * width + ix)*4] ? 1 : 0;
		if(bit == 1 && sel[sy*swidth+sx] == 1)
		{
                 buff[y*width+x] = 1;
	      }
	    }
	  }
      }
   }

   for(i=0;i<width*height;i++)
   {
     if(buff[i])
     {
        binary.data[i*4] = 255;
        binary.data[i*4+1] = 255;
        binary.data[i*4+2] = 255;
     }
     else
     {
        binary.data[i*4] = 0;
        binary.data[i*4+1] = 0;
        binary.data[i*4+2] = 0;
     }
   }
} 


function erode(binary, width, height, sel, swidth, sheight)
{
  var x, y;
  var sx, sy;
  var csx, csy;
  var ix, iy;
  var bit;
  var i;
  var buff = [];

  
  width = binary.width;
  height = binary.height;
  csx = Math.floor(swidth/2);
  csy = Math.floor(sheight/2);

  for(i=0;i<width*height;i++)
  {
     buff.push(1);
  }
  
   for(y=0;y<height;y++)
   {
      for(x=0;x<width;x++)
      {
        for(sy=0;sy<sheight;sy++)
        {
           for(sx =0; sx < swidth; sx++)
	    {
		  ix = x + sx - csx;
	        iy = y + sy - csy;
             if(ix < 0 || ix >= width || iy < 0 || iy >= height)
			bit = 0;
		else
		       bit = binary.data[(iy * width + ix)*4] ? 1 : 0;
		if(bit == 0 && sel[sy*swidth+sx] == 1)
		{
                 buff[y*width+x] = 0;
	      }
	    }
	  }
      }
   }

   for(i=0;i<width*height;i++)
   {
     if(buff[i])
     {
        binary.data[i*4] = 255;
        binary.data[i*4+1] = 255;
        binary.data[i*4+2] = 255;
     }
     else
     {
        binary.data[i*4] = 0;
        binary.data[i*4+1] = 0;
        binary.data[i*4+2] = 0;
     }
   }
} 
