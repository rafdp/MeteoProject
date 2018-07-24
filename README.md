# MeteoProject
3D visualization of atmospheric fronts.

This project was developed in contribution with [Гидрометцентр России](meteoinfo.ru) (ГМЦ). 
The task was to create a 3d visualization of athmospheric fronts based on a sliced front map provided by ГМЦ. 

The 3D visualization is based on Direct3D using wrappers, developed as part of the refraction project (https://github.com/rafdp/DirectXTest).
To build a 3D map of the fronts the Direct3D Texture3D was used. This object restores a 3D model from 2D slices, but has its downsides, 
as is only able to find straight vertical dependencies, while fronts can be slightly angled.

To display the fronts with right alpha channels and depth management, the ray marching algorithm was used. 

Following development by @Andrew-Bezzubtsev

[Thesises](https://www.dropbox.com/s/9ids7g1v5u7mni0/thesisesMeteo.doc?dl=0)
[Presentation](https://www.dropbox.com/s/pmm3djyxym2fdx4/Meteo.pdf?dl=0)
