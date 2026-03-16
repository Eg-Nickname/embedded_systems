# Program generujący graf zadań

Program należy uruchomić w konsoli przy użyciu:

```bash
./gen_task_graph.exe
```

Na wejściu w terminalu użytkownik zostaje poproszony o podanie (w przypadku podania danej w złym formacie program prosi o ponowne wprowadzenie danej):

- liczby zadań (>0)
- ilośc procesorów generlanego zastosowania (PP) (>0)
- ilośc jednostek sprzętowych (HC) (>0)
- ilość lini komunikacji (CL) (>0)
- czy wartości krawędzi mają być nie zerowe ("Y", "Yes", "N", "No", w przypadku błednej wersji zostanie wybrane domyslnie nie)
- nazwa pliku wyjściowego

Po podaniu informacji program zapisze wygenerowany graf zadań dla podanych parametrów.
