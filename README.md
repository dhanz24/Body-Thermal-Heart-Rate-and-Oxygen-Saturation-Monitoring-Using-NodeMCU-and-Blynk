# Body-Thermal-Heart-Rate-and-Oxygen-Saturation-Monitoring-Using-NodeMCU-and-Blynk
  Body Thermal, Heart Rate, and Oxygen Saturation Monitoring Using NodeMCU and Blynk, adalah sistem pemantauan detak jantung, saturasi oksigen, dan suhu pasien untuk mengatasi membludaknya pasien rumah sakit saat terjadinya pandemi Covid-19, sistem ini menggunakan NodeMCU, Sensor Oximeter MAX30105 dan juga Blynk.

  Alat ini menggunakan sensor Oximeter MAX30105 untuk mengukur detak jantung dan saturasi oksigen pada pasien, lalu untuk mengukur suhu tubuh menggunakan sensor Suhu IC LM35, sedangkan sebagai mikrokontroller nya menggunakan NodeMCU V3 ESP8266. Sensor Oximeter MAX30105 adalah jenis sensor yang digunakan dalam menghitung laju detak jantung dan saturasi oksigen seseorang secara non-invasive. Sensor ini mengandung 2 Light Emiting Dioda (LED) yaitu LED merah dan inframerah di dalamny, Adapun photodetector dengan pemrosesan sinyal analog noise rendah. Untuk dapat mengukur detak jantung dan saturasi oksigen pada pasien dapat menggunakan metode Photoplethysmography dalam penerapannya.
  
# ~Photoplethysmography
![image](https://github.com/dhanz24/Body-Thermal-Heart-Rate-and-Oxygen-Saturation-Monitoring-Using-NodeMCU-and-Blynk/assets/122549875/7dcc9c1e-b1ab-48ca-9e0e-990f9f361b81)
  Photoplethysmography atau PPG merupakan suatu metode yang digunakan untuk mengukur perubahan volume darah pada pembuluh darah yang terdapat dekat dengan lapisan kulit manusia berbasis optik. Metode ini sudah banyak diaplikasikan pada alat-alat medis seperti pulse-oximeter, diagnostic vascular, denyut nadi, dan tekanan darah. Prinsip kerja PPG memerlukan beberapa komponen sumber cahaya yang panjang gelombangnya dapat disesuaikan dengan kebutuhan dan sebuah photodetector untuk merubah perubahan radiasi gelombang cahaya menjadi sama dengan perubahan volume darah.


# ~Diagram Blok sistem
![image](https://github.com/dhanz24/Body-Thermal-Heart-Rate-and-Oxygen-Saturation-Monitoring-Using-NodeMCU-and-Blynk/assets/122549875/a37a9127-7d5e-40ea-986f-fd46beda9409)
diilustrasikan bahwa perangkat monitoring mulai bekerja ketika sensor suhu IC LM 35DZ yang diarahkan langsung ke pergelangan tangan menghitung suhu tubuh pengguna dan sensor oximeter MAX30105 yang diarahkan langsung ke jari mengukur detak jantung juga saturasi oksigen. Selanjutnya data dari sensor akan diteruskan dan diproses oleh NodeMCU. Setelah NodeMCU mengolah data yang diterimanya melalui sensor, maka data tersebut akan dikirimkan oleh wifi ESP8266 ke router lalu setelahnya ke internet dan berakhir diterima oleh server Blynk. Informasi ini akan diteruskan kembali ke aplikasi Blynk pengguna yang akan memonitoring perangkat. Ketika suhu tubuh, detak jantung, dan saturasi oksigen terdeteksi tidak normal maka aplikasi Blynk akan mengirimkan notifikasi dan email ke kontak yang terhubung.

# ~Diagram sirkuit yang digunakan dalam sistem
![image](https://github.com/dhanz24/Body-Thermal-Heart-Rate-and-Oxygen-Saturation-Monitoring-Using-NodeMCU-and-Blynk/assets/122549875/3b078b32-fdb2-472a-bb50-1f24ff4f9b1a)
Pada gambar dapat dilihat bahwa sensor MAX30105 dan sensor IC LM 35DZ berinteraksi langsung dengan pengguna, sedangkan NodeMCU V3 ESP8266MOD berfungsi menghubungkan langsung perangkat ke router dan internet. Digambarkan bahwa pin Vin pada sensor MAX30105 terhubung pada pin 3v NodeMCU, pin SDA ke D1 NodeMCU, pin SCl ke pin D2 NodeMCU, dan pin ground ke ground NodeMCU. Pada sensor IC LM 35DZ sendiri digambarkan bahwa pin Vin terhubung pada pin 3V nodemcu, ground ke ground nodemcu, pin tengah sebagai Analog ke A0 NodeMCU

# ~Contoh Penerapan Sistem
![image](https://github.com/dhanz24/Body-Thermal-Heart-Rate-and-Oxygen-Saturation-Monitoring-Using-NodeMCU-and-Blynk/assets/122549875/9c4c5a5b-4cfa-4e4b-810b-345f4d71f24a)![image](https://github.com/dhanz24/Body-Thermal-Heart-Rate-and-Oxygen-Saturation-Monitoring-Using-NodeMCU-and-Blynk/assets/122549875/ff146be1-db21-4500-8d41-1907fb08ce26)

![image](https://github.com/dhanz24/Body-Thermal-Heart-Rate-and-Oxygen-Saturation-Monitoring-Using-NodeMCU-and-Blynk/assets/122549875/7f063749-fca4-4275-b53d-bc8b28ca5704)

![image](https://github.com/dhanz24/Body-Thermal-Heart-Rate-and-Oxygen-Saturation-Monitoring-Using-NodeMCU-and-Blynk/assets/122549875/8472f654-37f3-4dd8-b144-2e7f450c902b)


![image](https://github.com/dhanz24/Body-Thermal-Heart-Rate-and-Oxygen-Saturation-Monitoring-Using-NodeMCU-and-Blynk/assets/122549875/dbd2ef9d-60d2-45cd-a641-8421e4ddaa3f)
