

Struktura katalogów

Podkatalogi zawierają:
server/ -- faktyczna aplikacja serwera dodatków
game/ -- interfejs gry (zmodyfikowana wersja Wesnoth 1.8)
script/ -- interfejst skryptu (zmodyfikowana wersja z Wesnoth 1.8)
wesnoth/ -- wspólne dla skryptu i serwera pakiety pythonowe
test_data/ -- dane przykładowe (testowe)


Serwer i interfejs WWW

Aby uruchomić serwer należy:
 - Zainstalować implementację języka Python w wersji 2.6 
   ( http://www.python.org/download/ )
 - Zainstalować platformę (framework) Django w wersji 1.1
   ( http://www.djangoproject.com/download/ )
Następnie:
 - Przejść do katalogu z projektem w linii poleceń i następnie do
   podkatalogu "server"
 - Przygotować bazę danych na jeden z dwóch sposobów:
   a) skopiować testową bazę danych o nazwie "test.db" z test-data do pliku
      "sqlite.db" w katalogu "server"
   b) utworzyć pustą bazę danych wykorzystując komendę: 
      "python manage.py syncdb"
 - Wpisać komendę "python manage.py runserver"
 - Skierować przeglądarkę internetową na adres http://localhost:8000

Uwaga: serwer zakłada obecność działających programów tar i bzip2 w
domyślnej ścieżce, co może wymagac dodatkowych zabiegów na systemach typu
Windows. W razie braku tych programów publikacja nie będzie działać
poprawnie. 


Interfejs skryptu

W celu skorzystania z intefrejcu skryptowego należy uruchomić skrypt Pythona
o nazwie "wesnoth_adddon_manager" w podkatalogu "script". Możliwe działania
wyjaśnia wywołanie skryptu z parametrem --help.


Interfejs gry

W celu skorzystania z interfejsu gry należy skompilować i uruchomić grę,
źródła znajdują się w podkatalogu ``game''. Do kompilacji potrzebny jest
program cmake lub scons oraz wersje deweloperskie wielu bibliotek, takich
jak sdl (libsdl*dev), boost, lua (liblua5.1-dev). Program scons sprawdza
obecność wymaganych bibliotek i powiadamia o tym, czego brakuje.

Dodatkowymi zależnościami w stosunku do bazowej wersji gry jest biblioteka 
curl (np. pakiet libcurl4-openssl-dev) oraz boost_thread.

Po skompilowaniu gry np. poleceniem "scons wesnoth" należy uruchomić plik
wykonywalny "wesnoth" po czym wybrać w menu głównym opcję "Add-ons" lub 
"Dodatki".


Dane przykładowe

Login i hasło do panelu administracyjnego: admin:admin
Login i hasło do publikacji/usuwania dodatków: autor([1-3]):autor\1

Przykładowe dodatki gotowe do publikacji znajdują się w podkatalogu
test_data/game_publish_test, aby umożliwić grze publikację należy wybrane
podkatalogi z tej lokalizacji umieścić w podkatalogu data/add-ons katalogu
danych użytkownika ("userdata"), na systemach Linux jest to domyślnie
katalog $HOME/.wesnoth lub $HOME/.wesnoth1.8. Gra przy uruchamianiu wypisuje
na standardowe wyjście błedu informację o tym, z jakiego katalogu faktycznie
korzysta.
