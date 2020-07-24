cd .\Build
java -jar st_decrypt.jar --key " .ST-Link.ver.3." -i STLINKv3_YAB.bin  -o f3_1.bin --encrypt
java -jar st_decrypt.jar --key " .ST-Link.ver.3." -i STLINKv3_YAB.bin  -o f3_2.bin --encrypt
