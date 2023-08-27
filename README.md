Program `rarjpeg_detector` determines whether the provided file is a rarjpeg or a regular archive or not an archive at all. If the file is a rarjpeg or a regular archive then the program prints its content.

To compile the program use (inside the project folder):

Linux:

```bash
$ gcc ./rarjpeg_detector.c -o ./rarjpeg_detector -Wall -Wextra -Wpedantic -std=c11
```
Windows:

```bash
$ gcc .\rarjpeg_detector.c -o .\rarjpeg_detector -Wall -Wextra -Wpedantic -std=c11
```

To run the program use (inside the project folder):

Linux:

```bash
$ ./rarjpeg_detector.exe ./test_files\<filename>
$ ./rarjpeg_detector.exe ./test_files\zipjpeg1.jpg
$ ./rarjpeg_detector.exe ./test_files\zipjpeg2.jpg
$ ./rarjpeg_detector.exe ./test_files\zipjpeg3.jpg
$ ./rarjpeg_detector.exe ./test_files\zipjpeg4.jpg
$ ./rarjpeg_detector.exe ./test_files\zippng.png
$ ./rarjpeg_detector.exe ./test_files\zippng.zip
$ ./rarjpeg_detector.exe ./test_files\non-zipjpeg1.jpg
$ ./rarjpeg_detector.exe ./test_files\non-zipjpeg2.jpg
```

Windows:

```bash
$ .\rarjpeg_detector.exe .\test_files\<filename>
$ .\rarjpeg_detector.exe .\test_files\zipjpeg1.jpg
$ .\rarjpeg_detector.exe .\test_files\zipjpeg2.jpg
$ .\rarjpeg_detector.exe .\test_files\zipjpeg3.jpg
$ .\rarjpeg_detector.exe .\test_files\zipjpeg4.jpg
$ .\rarjpeg_detector.exe .\test_files\zippng.png
$ .\rarjpeg_detector.exe .\test_files\zippng.zip
$ .\rarjpeg_detector.exe .\test_files\non-zipjpeg1.jpg
$ .\rarjpeg_detector.exe .\test_files\non-zipjpeg2.jpg
```