Podkatalogi zawierają:
server/ -- faktyczna aplikacja serwera dodatków
game/ -- interfejs gry (zmodyfikowana wersja Wesnoth 1.8)
script/ -- interfejst skryptu (zmodyfikowana wersja z Wesnoth 1.8)
wesnoth/ -- wspólne dla skryptu i serwera pakiety pythonowe
test_data/ -- dane przykładowe (testowe)

Aby uruchomić prototyp należy:
 - Zainstalować implementację języka Python w wersji 2.6 ( http://www.python.org/download/ )
 - Zainstalować platformę (framework) Django w wersji 1.1.1 ( http://www.djangoproject.com/download/ )
Następnie:
 - Przejść do katalogu z prototypem w linii poleceń
 - Przygotować bazę danych na jeden z dwóch sposobów:
   a) skopiować testową bazę danych o nazwie "test.db", do pliku "sqlite.db"
   b) utworzyć pustą bazę danych wykorzystując komendę: "python manage.py syncdb"
 - Wpisać komendę ,,python manage.py runserver''
 - Skierować przeglądarkę internetową na adres http://localhost:8000

Login i hasło do panelu administracyjnego: admin:admin
Login i hasło do interfejsu prototypowego publikacji/usuwania dodatków: autor([1-3]):autor\1
