#!/usr/bin/python3
from tkinter import *
import sys
import math
import time

BUFSIZE = 188*1
w = 1280
h = 1280
pcolor = "#20ff00"

def make_streams_binary():
    sys.stdin = sys.stdin.detach()
    #sys.stdout = sys.stdout.detach()

def _draw_grid(self,w,h,**kwargs):
    self.create_line(0,h/2,w,h/2,**kwargs)
    self.create_line(w/2,0,w/2,h,**kwargs)
    if (h>w):
        fac = 2*w/256
    else:
        fac = 2*h/256
    for n in range (0,int(w/2),int(5*fac)):
        self.create_line(n+w/2,h/2-fac,n+w/2,h/2+fac,**kwargs)
        self.create_line(w/2-n,h/2-fac,w/2-n,h/2+fac,**kwargs)
    for n in range (0,int(h/2),int(5*fac)):
        self.create_line(w/2-fac,n+h/2,w/2+fac,n+h/2,**kwargs)
        self.create_line(w/2-fac,h/2-n,w/2+fac,h/2-n,**kwargs)   
Canvas.draw_grid = _draw_grid
    
def _draw_data(self,w,h,**kwargs):
    data = sys.stdin.read(BUFSIZE)
    self.delete("all")
    self.draw_grid(w,h,fill="yellow")
    for i in range(0, BUFSIZE, 188):
        for n in range(4, 187, 2):
            if (h>w):
                fac = w/256
            else:
                fac = h/256
            p = int(data[n+i])
            q = int(data[n+1+i])
            if (p > 127):
                p = (p &0x7f)-128
            if (q > 127):
                q = (q &0x7f)-128
            q=-q
            s = 3
            x1, y1 = (p*fac - s)+w/2, (q*fac - s)+h/2
            x2, y2 = (p*fac + s)+w/2, (q*fac + s)+h/2
            self.create_oval(x1, y1, x2, y2, **kwargs)
Canvas.draw_data = _draw_data

def draw_loop(mycanvas):
    global w
    global h
    mycanvas.draw_data(w,h,fill=pcolor)
    mycanvas.after(1,draw_loop,mycanvas)

def main():
    def clear(event):
        global w
        global h
        canvas.delete("all")
        canvas.draw_grid(w,h,fill="yellow")

    def close(event):
        root.withdraw()
        root.destroy()

    def on_resize(event):
        global w
        global h
        nh = event.height
        nw = event.width
        if (abs(nw-w)+abs(nh-h)>0):
            w = nw
            h = nh
            canvas.delete("all")
            canvas.config(width=w,height=h)
            canvas.draw_grid(w,h,fill="yellow")
        
    root = Tk()
    root.bind('<Escape>', close)
    root.bind('q', close)
    root.resizable(True, True) 
    make_streams_binary()

    canvas = Canvas(root, width=w, height=h, borderwidth=0, highlightthickness=0, bg="black")
    root.bind('c', clear,canvas)
    canvas.bind("<Configure>",on_resize)
    canvas.pack(fill="both", expand=True)
    canvas.draw_grid(w,h,fill="yellow")
    draw_loop(canvas)
    
    root.wm_title("DVB IQ")
    root.mainloop()

if __name__ == "__main__":
    main()
      
