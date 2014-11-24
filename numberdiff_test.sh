sed 's/^\(-*[0-9]*\.[0-9]\)$/\10/g' < ty.txt | sed 's/^\(-*[0-9]*\)$/\1.00/g' | sed 's/\.//g' > ty_fixed.txt
./numberdiff c compressed_ty.temp < ty_fixed.txt
./numberdiff d compressed_ty.temp n > decompressed_ty.temp

tr -d '.' < curve1.dat > curve1_fixed.temp
./numberdiff c compressed_curve1.temp < curve1_fixed.temp
./numberdiff d compressed_curve1.temp y > decompressed_curve1.temp
