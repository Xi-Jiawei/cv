objects = utils.o bmp.o tiff.o edgedetect.o

test: $(objects)
	gcc -o main main.c $(objects) -lm -g

utils.o: utils.c
	gcc -o utils.o -c utils.c -g
bmp.o: bmp.c
	gcc -o bmp.o -c bmp.c -g
tiff.o: tiff.c
	gcc -o tiff.o -c tiff.c -g
edgedetect.o: edgedetect.c
	gcc -o edgedetect.o -c edgedetect.c -g

clean:
	rm main $(objects)
