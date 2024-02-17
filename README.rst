.. _mesh_badge:

Mesh Badge DE
#############

Deutsch - Übersicht
*******************

Dies ist die deutsche Version für die Anwendung "mesh_badge" für das
Phytec "reel board". Die Texte für das ePaper sind ins Deutsche 
übersetzt, die Debug-Infos nur teilweise. Die Funktion Meshen wurde 
geringfügig erweitert. Anstelle "Saying HI to everyone" erfolgt nun
mit eingefügtem Sendername "<Name> sagt "Hallo" zu Jedem!".

Das Projekt hat Beta-Status und ist so anzunehmen wie es ist. Die 
Anwendung der Software erfolgt auf eigenem Risiko.


Diese Applikation für das "reel board" zeigt beispielhaft das Bluetooth 
untereinander Verbinden (meshen).

Diese App wurde ursprünglich als eine Bluetooth GATT Anwendung entwickelt.
Installieren Sie die "nRF Connect" App auf Ihr Smartphone (verfügbar 
für Android und iOS) um den Dienst mit dem "reel board" nutzen zu 
können. Mit einem PC kann ebenso mit jedem Bluetooth LE GATT client 
auf den Dienst zugegriffen werden. Diese Hinweise hier beziehen
sich auf die notwendigen Schritte für Mobiles.

Bedienung
*********

#. Nutzen Sie die nRF Connect App für die Gerätesuche und schauen 
   Sie nach "reel board".
#. Verbinden Sie sich mit dem Gerät. Es ist ein Dienst verfügbar.
   Wählen Sie ihn aus.
#. Durch Drücken der "Pfeil nach oben" Taste wählen Sie "Schreiben" aus.
#. Wählen Sie "Text" anstelle von HEX-Eingabe aus.
#. Geben Sie ihren Namen ein (oder einen anderen Text). Mehrere Worte
   getrennt durch Leerzeichen sind möglich. Die Zeichengröße auf dem 
   reel board erlaubt 3 Spalten mit je 12 Zeichen. Einen Zeilenumbruch kann
   mit einem Komma erzwungen werden.
#. Drücken Sie "Send". Im nächsten Schritt erfolgt das Pairing. Der Passkey
   wird auf dem Board angezeigt. Übernehmen Sie diesen auf Ihr Smartphone.
#. Wenn das Pairing erfolgt ist, wird der gesendete Text angezeigt. Wenn Sie
   noch etwas am Text ändern wollen, können Sie dieses nun tun.
#. Wenn alles ok ist, trennen das Board (beenden Sie die App oder gehen
   Sie auf die Scann Seite ihres Phones.
#. Einmal getrennt, schaltet das Board in den Bluetooth Mesh Mode. Von da an
   kann man das Board nicht mehr über GATT angesprechen.
   
Wenn man mehrere Boards wie dieses konfiguriert, kommunizieren dies untereinander 
via mesh: durch Drücken des "user" Knopfes auf dem Board wird das erste Wort 
(Name) von dem gespeicherten Text von allen anderen Boards im Netzwerk 
empfangen und fordert dort die Anzeige des Textes "<Name> sagt Hallo!".

Um das Board in seinen initialen Zustand zurückzusetzen (Abschalten von mesh, 
Löschen des gespeicherten Textes und Wiederansprechen über GATT) sind folgende
Schritte notwendig:

#. Beim Einschalten den "User"-Knopf gedrückt halten (oder gemeinsam mit dem
   "Reset"-Knopf falls eingeschaltet).
#. Warten, bis "Reseting Device" angezeigt wird.



Die Original-Software (Master) stammt von
https://github.com/zephyrproject-rtos/zephyr/tree/master/samples/boards/reel_board/mesh_badge

Um diese für das "reel board", auch in der Deutschen Version kompiliernen zu können,
benötigen Sie die ZEPHYR Entwicklungsumgebung. Weitere Infos hierzu finden Sie unter 
https://docs.zephyrproject.org/latest/boards/arm/reel_board/doc/reel_board.html




English - Overview
******************

This sample app for the reel board showcases Bluetooth Mesh

The app starts off as a regular Bluetooth GATT peripheral application.
Install the the "nRF Connect" app on your phone (available both for
Android and iOS) to access the service that the app exposes. The service
can also be accessed with any Bluetooth LE GATT client from your PC,
however these instructions focus on the necessary steps for phones.

Steps to set up
***************

#. On your phone, use the nRF Connect app to Scan for devices and look
   for "reel board"
#. Connect to the device. You'll see a single service - select it
#. Request to write to the characteristic by pressing on the upward pointing
   arrow symbol
#. Select "Text" to enter text instead of hex
#. Enter your name (or any other arbitrary text). Multiple words
   separated by spaces are possible. The font used on the reel display
   allows three rows of up to 12 characters
   wide text. You can force line breaks with a comma.
#. Press "Send" - this will trigger pairing since this is a protected
   characteristic. The passkey for the pairing will be shown on the board's
   display. Enter the passkey in your phone.
#. Once pairing is complete the board will show the text you sent. If
   you're not happy with it you can try writing something else.
#. When you're happy with the text, disconnect from the board (exit the app or
   go back to the device scan page)
#. Once disconnected the board switches over to Bluetooth Mesh mode, and you
   can't connect to it anymore over GATT.

If you configure multiple boards like this they can communicate with
each other over mesh: by pressing the user button on the board the first
word (name) of the stored text will be sent to all other boards in
the network and cause the other boards to display "<name> says hi!".

To reset a board to its initial state (disable mesh, erase the stored
text, and make it connectable over GATT):

#. Keep the user button pressed when powering on (or press the reset button
   when powered)
#. Wait until "Reseting Device" is shown
