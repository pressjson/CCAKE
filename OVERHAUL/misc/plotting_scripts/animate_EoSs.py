import h5py
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import sys

f = h5py.File(sys.argv[1], 'r')
event = f['Event']
n_timesteps = len(event.keys())
event_keys = list(event.keys())


def init():
    frame = event[event_keys[0]]
    x = np.array(frame['x'])
    y = np.array(frame['y'])
    im = plt.scatter(x, y)
    return im,


def animate(i):
    print('Plotting frame', i)
    frame = event[event_keys[i]]
    x = np.array(frame['x'])
    y = np.array(frame['y'])
    im = plt.scatter(x, y, c = np.array(frame['e']), s = 0.001,
                     cmap = cm.get_cmap('inferno', 256) )
    
    #plt.imsave(fname='old_animation_frames/frame' + str(i) + '.png', \
    #           arr=image, cmap=chosen_colormap, format='png')
    return im,



def main():

    # Plot Volume Rendering
    fig = plt.figure(figsize=(5,5), dpi=50)
    plt.subplots_adjust(top=1, bottom=0, right=1, left=0, hspace=0, wspace=0)
    plt.margins(0, 0)
        
    # Do Volume Rendering at Different Viewing Angles
    ani = animation.FuncAnimation(fig, animate, np.arange(n_timesteps), \
                                  init_func=init, blit=True)

    out = "EoS_particle_evolution.mp4"
    print('Saving to', out)
    FFwriter = animation.FFMpegWriter(fps=10, extra_args=['-vcodec', 'libx264'])
    ani.save(out, writer=FFwriter)
    print('Finished everything.')
    #out = "animation.gif" 
    #ani.save(out, writer='imagemagick', fps=10)

    return 0

  
if __name__== "__main__":
  main()

