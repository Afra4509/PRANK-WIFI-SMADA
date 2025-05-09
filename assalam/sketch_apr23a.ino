#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <SPIFFS.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer webServer(80);

// Nama SSID WiFi yang akan ditampilkan
const char* ssid = "SmadaFreeWifi";

// File audio yang akan diputar
const char* audioFileName = "/laugh.mp3";

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  // Buat Access Point WiFi
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  
  Serial.println("Access Point dibuat");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
  
  // Aktifkan DNS Server
  dnsServer.start(DNS_PORT, "*", apIP);
  
  // Rute untuk halaman login palsu
  webServer.on("/", handleRoot);
  webServer.on("/audio", handleAudio);
  
  // Tangani rute yang tidak ada
  webServer.onNotFound(handleRoot);
  
  // Mulai web server
  webServer.begin();
  
  Serial.println("Server berhasil dijalankan");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

void handleRoot() {
  String html = R"(
    <!DOCTYPE html>
    <html lang="id">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>SmadaFreeWifi</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #f0f0f0;
                margin: 0;
                padding: 0;
                display: flex;
                justify-content: center;
                align-items: center;
                height: 100vh;
                color: #333;
            }
            .hotspot-container {
                background-color: white;
                border-radius: 12px;
                box-shadow: 0 2px 15px rgba(0, 0, 0, 0.1);
                padding: 25px;
                width: 90%;
                max-width: 350px;
                text-align: center;
            }
            .logo {
                width: 100px;
                height: 100px;
                margin: 0 auto 20px;
                background-color: #0078d7;
                border-radius: 50%;
                display: flex;
                justify-content: center;
                align-items: center;
                color: white;
                font-size: 28px;
                font-weight: bold;
            }
            h1 {
                color: #0078d7;
                margin-bottom: 15px;
                font-size: 24px;
            }
            p {
                color: #666;
                margin-bottom: 25px;
                font-size: 16px;
                line-height: 1.5;
            }
            .connect-btn {
                background-color: #0078d7;
                color: white;
                border: none;
                padding: 14px 0;
                width: 100%;
                border-radius: 24px;
                cursor: pointer;
                font-size: 18px;
                font-weight: bold;
                transition: background-color 0.3s;
            }
            .connect-btn:hover {
                background-color: #0067b8;
            }
            .disclaimer {
                font-size: 12px;
                color: #888;
                margin-top: 25px;
            }
            .hidden {
                display: none;
            }
        </style>
    </head>
    <body>
        <div class="hotspot-container">
            <div class="logo">SMADA</div>
            <h1>SmadaFreeWifi</h1>
            <p>Selamat datang di WiFi gratis SMADA. Klik tombol di bawah untuk terhubung ke internet.</p>
            <button id="connectBtn" class="connect-btn">Hubungkan ke Internet</button>
            <p class="disclaimer">Dengan menghubungkan, Anda menyetujui syarat dan ketentuan penggunaan WiFi gratis ini.</p>
        </div>
        
        <audio id="prankAudio" loop>
            <source src="/audio" type="audio/mpeg">
            Your browser does not support the audio element.
        </audio>

        <script>
            // Fungsi untuk memainkan audio
            function playAudio() {
                const audio = document.getElementById('prankAudio');
                audio.volume = 1.0;
                
                const playPromise = audio.play();
                
                if (playPromise !== undefined) {
                    playPromise.then(_ => {
                        // Mengubah tampilan ketika audio berhasil diputar
                        document.querySelector('.hotspot-container').innerHTML = '<h1>Koneksi Berhasil!</h1><p>Anda telah terhubung ke SmadaFreeWifi!</p><p>Silahkan nikmati internet gratis.</p>';
                        console.log("Audio berhasil diputar");
                    })
                    .catch(error => {
                        console.log("Gagal memutar audio:", error);
                        // Mencoba lagi dengan interaksi pengguna
                        alert("Klik OK untuk melanjutkan koneksi");
                        audio.play().catch(e => console.log("Masih gagal setelah alert:", e));
                    });
                }
            }
            
            // Jalankan audio saat tombol connect ditekan
            document.getElementById('connectBtn').addEventListener('click', function(e) {
                e.preventDefault();
                playAudio();
            });
            
            // Konfirmasi saat user mencoba menutup halaman
            window.addEventListener('beforeunload', function(e) {
                e.preventDefault();
                e.returnValue = 'Yakin mau keluar?';
                return 'Yakin mau keluar?';
            });
            
            // Coba cegah pengguna menutup tab dengan berbagai metode
            window.addEventListener('pagehide', function() {
                navigator.vibrate([200, 100, 200]);
            });
            
            document.addEventListener('visibilitychange', function() {
                if (document.visibilityState === 'hidden') {
                    navigator.vibrate([200, 100, 200]);
                }
            });
            
            // Membuat audio tetap bermain saat user mencoba navigasi balik
            window.history.pushState(null, null, window.location.href);
            window.addEventListener('popstate', function() {
                window.history.pushState(null, null, window.location.href);
            });
        </script>
    </body>
    </html>
  )";
  
  webServer.send(200, "text/html", html);
}

void handleAudio() {
  File file = SPIFFS.open(audioFileName, "r");
  if (!file) {
    webServer.send(404, "text/plain", "File audio tidak ditemukan");
    return;
  }
  
  webServer.streamFile(file, "audio/mpeg");
  file.close();
}