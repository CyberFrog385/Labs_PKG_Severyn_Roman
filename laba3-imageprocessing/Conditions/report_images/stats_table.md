| Операция | Изображение | До | После | График |
|---|---|---|---|---|
| Линейное контрастирование (авто) | low_contrast_home.png | min=87 max=150 среднее=115.7 sigma=11.4 | min=0 max=253 среднее=116.0 sigma=46.1 | ![](report_images/hist_low_contrast_home_contrast-auto.png) |
| Эквализация по каналам RGB | low_contrast_home.png | min=87 max=150 среднее=115.7 sigma=11.4 | min=0 max=255 среднее=130.4 sigma=62.9 | ![](report_images/hist_low_contrast_home_equalize-rgb.png) |
| Эквализация яркости HSV | low_contrast_home.png | min=87 max=150 среднее=115.7 sigma=11.4 | min=0 max=254 среднее=120.5 sigma=70.8 | ![](report_images/hist_low_contrast_home_equalize-hsv.png) |
| CLAHE (тайлы 8x8, лимит 2) | low_contrast_home.png | min=87 max=150 среднее=115.7 sigma=11.4 | min=65 max=177 среднее=122.1 sigma=18.0 | ![](report_images/hist_low_contrast_home_clahe.png) |
| Линейное контрастирование (авто) | low_contrast_butterfly.png | min=84 max=148 среднее=112.7 sigma=17.6 | min=0 max=255 среднее=114.4 sigma=70.1 | ![](report_images/hist_low_contrast_butterfly_contrast-auto.png) |
| Эквализация по каналам RGB | low_contrast_butterfly.png | min=84 max=148 среднее=112.7 sigma=17.6 | min=0 max=255 среднее=127.5 sigma=72.1 | ![](report_images/hist_low_contrast_butterfly_equalize-rgb.png) |
| Эквализация яркости HSV | low_contrast_butterfly.png | min=84 max=148 среднее=112.7 sigma=17.6 | min=0 max=255 среднее=124.9 sigma=71.6 | ![](report_images/hist_low_contrast_butterfly_equalize-hsv.png) |
| CLAHE (тайлы 8x8, лимит 2) | low_contrast_butterfly.png | min=84 max=148 среднее=112.7 sigma=17.6 | min=62 max=184 среднее=115.5 sigma=28.3 | ![](report_images/hist_low_contrast_butterfly_clahe.png) |
| Эквализация яркости HSV на зашумленном | noisy_gauss_home.png | min=0 max=237 среднее=119.1 sigma=35.2 | min=0 max=253 среднее=101.5 sigma=66.5 | ![](report_images/hist_noisy_gauss_home_equalize-hsv.png) |
| CLAHE на зашумленном (лимит 4) | noisy_gauss_home.png | min=0 max=237 среднее=119.1 sigma=35.2 | min=0 max=253 среднее=105.6 sigma=56.3 | ![](report_images/hist_noisy_gauss_home_clahe.png) |
| Гамма-коррекция 0.5 | dark_fruits.png | min=0 max=95 среднее=35.1 sigma=18.3 | min=0 max=156 среднее=88.1 sigma=30.4 | ![](report_images/hist_dark_fruits_gamma.png) |
| Линейное контрастирование (авто) | dark_fruits.png | min=0 max=95 среднее=35.1 sigma=18.3 | min=0 max=242 среднее=89.4 sigma=46.6 | ![](report_images/hist_dark_fruits_contrast-auto.png) |
