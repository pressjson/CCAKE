import h5py as h5
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib import cm                                                                          
from matplotlib.colors import ListedColormap, LinearSegmentedColormap, Normalize
import numpy as np
import sys
from mpl_toolkits.axes_grid1 import make_axes_locatable


# HDF file structure
f = h5.File(sys.argv[1], 'r')
event = f['Event']
n_timesteps = min([len(event.keys()),1000])
event_keys = list(event.keys())

# Figure dimensions
width, height = 6, 6

# Axes ranges
xmin, xmax, ymin, ymax = 2.5, 7.5, -7.5, -2.5
#xmin, xmax, ymin, ymax = -12, 12, -12, 12

chosen_colormap = cm.get_cmap('cool', 256)

chosen_dpi = 200

labels = {'e': r'$e$ (MeV/fm$^3$)', 'B': r'$\rho_B$ (fm$^{-3}$)',
          'S': r'$\rho_S$ (fm$^{-3}$)', 'Q': r'$\rho_Q$ (fm$^{-3}$)'}

#########################################################################################
def frame_to_array(i, quantity):
    frame = event[event_keys[i]]
    q = np.array(frame[quantity])
    return np.c_[ np.full_like(q, frame.attrs['Time']), q, np.array(frame['e']) ]


#########################################################################################
def plot_density_distribution_vs_time(quantity):
    # Set up
    print('Building data...')
    data = np.stack([frame_to_array(i, quantity) for i in range(n_timesteps)])
    
    #print(data.shape)
    
    data = data.reshape([data.size//3,3])
    
    #print(np.amin(data[:,0]), np.amax(data[:,0]))
    
    ti, tf = np.amin(data[:,0]), np.amax(data[:,0])
    dt = (tf - ti) / (n_timesteps - 1.0)
    #print(ti, tf, tf - ti, n_timesteps, dt)
    timebins = np.arange(ti-dt, tf+dt, dt)
    #timebins = np.arange(0.55,13.15,0.05)
    timebins = 0.5*(timebins[1:]+timebins[:-1])
    
    # freeze-out cutoff [MeV/fm^3]
    eFO = 266.0
    data = data[ data[:,2] >= eFO ]
    
    # only look at positive densities, for simplicity
    data = data[ data[:,1] >= 1e-6 ]
    
    H, xedges, yedges = np.histogram2d(data[:,0], np.log(data[:,1]), bins=[timebins,250])
        
    #####################################
    # T vs. muB figure
    #####################################
    print('Plotting...')
    plt.figure(figsize=(width, height), dpi=chosen_dpi)
    
    #print(xedges.shape, yedges.shape, H.shape)

    #plt.pcolormesh(yedges, xedges, H.T, cmap='inferno')
    plt.imshow(H.T, cmap='inferno', interpolation='bicubic', origin='lower',\
               extent=[np.amin(xedges),np.amax(xedges),\
                       np.exp(np.amin(yedges)),np.exp(np.amax(yedges))])
    
    plt.xlabel(r'$\tau$ (fm/$c$)')
    plt.ylabel(labels[quantity])
    
    outfilename = quantity + '_vs_tau.png'
    plt.savefig(outfilename, dpi=chosen_dpi, bbox_inches='tight', pad_inches = 0)
    print('Saved to ' + outfilename)



#########################################################################################
if __name__== "__main__":
    plot_density_distribution_vs_time('e')
    plot_density_distribution_vs_time('B')
    plot_density_distribution_vs_time('S')
    plot_density_distribution_vs_time('Q')



