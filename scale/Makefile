objects = utils.o bmp.o tiff.o scale.o bits.o scaleraw.o demosaic.o

test: $(objects)
	gcc -o main main.c $(objects) -lm -g

utils.o: utils.c
	gcc -o utils.o -c utils.c -g
bmp.o: bmp.c
	gcc -o bmp.o -c bmp.c -g
tiff.o: tiff.c
	gcc -o tiff.o -c tiff.c -g
bits.o: bits.c
	gcc -o bits.o -c bits.c -g
demosaic.o: demosaic.c
	gcc -o demosaic.o -c demosaic.c -g
scale.o: scale.c
	gcc -o scale.o -c scale.c -g
scaleraw.o: scaleraw.c
	gcc -o scaleraw.o -c scaleraw.c -g

clean:
	rm main $(objects)
