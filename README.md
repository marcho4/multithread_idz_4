# multithread_idz_4
## Вариант 10
**Задача о курильщиках.** <br>
Есть три потока–курильщика и один поток–посредник. 
Курильщик непрерывно скручивает сигареты и курит их.
Чтобы скрутить сигарету,нужны табак,бумага и спички.
У одного курильщика есть табак, у второго — бумага, а у третьего – спички. 
Посредник через некоторое случайное время кладет на стол по два разных случайных компонента. 
Тот Курильщик, у которого есть третий компонент, забирает компоненты со стола, скручивает сигарету и курит некоторое случайное время. 
Посредник через отведенный ему интервал времени, который может быть больше или меньше времени курения, выкладывает следующий набор. 
Если курильщик, которому нужен этот набор, свободен, то он начинает курить. 
Если курильщик еще курит, то процесс передачи дожидается окончания курения. Затем процесс повторяется. 
В принципе возможна ситуация, когда все три курильщика могут курить одновременно или все могут ждать, когда посредник соизволит выложить очередной набор. 
Создать многопоточное приложение, моделирующее поведение курильщиков и посредника.

## Отчет
Используется паттерн Издатель - Подписчики <br>
В main.cpp присутствуют комментарии к строчкам кода, объясняющие логику действий.<br>
Издатель выкладывает на стол предметы с интервалом от 1 до 10 секунд. <br>
Курильщик (Подписчик) курит от 1 до 10 секунд. <br>
Если он занят, а задание для него появилось, то оно кладется в очередь заданий. Такая очередь существует для каждого потока.<br>
Программа записывает вывод в файл, имя файла можно передать как аргумент командой строки при запуске программы.<br>

### Сценарий работы программы
У нас есть 4 мьютекса (1 главный и по одному для каждого подписчика).<br>
Также есть 3 condition_variable для каждого подписчика, аналогично мьютексам.<br>
В главном потоке генерируется случайное число, оно конвертируется в компонент, которого не хватает для полной сигареты.<br>
Соответственно, после генерации компонента, мы оповещаем condition_variable этого компонента, о том, что произошли изменения в программе.<br>
Когда поток получает сообщение, он проверяет свою очередь заданий на пустоту, и если там есть какое-то задание - приступает к работе. <br>
Также это задание удаляется из очереди, при этом блокируется главный мьютекс, для корректного освобождения памяти. (Издатель аналогично блокирует главный mutex, когда кладет задание в очередь потока)<br>
Мьютекс данного воркера блокируется, после выполнения работы, он разлачивается.<br>

--
Пример вывода в файл:
--
Component missing: paper
Smoker with paper started smoking for 7 seconds
Component missing: spichki
Smoker with spichki started smoking for 9 seconds
Smoker with paper finished smoking
Component missing: spichki
Smoker with spichki finished smoking
Smoker with spichki started smoking for 8 seconds
Component missing: paper
Smoker with paper started smoking for 6 seconds
Smoker with spichki finished smoking
Smoker with paper finished smoking
Component missing: spichki
Smoker with spichki started smoking for 7 seconds
Smoker with spichki finished smoking
Component missing: spichki
Smoker with spichki started smoking for 6 seconds
Smoker with spichki finished smoking
Component missing: tabak
Smoker with tabak started smoking for 9 seconds
Component missing: spichki
Smoker with spichki started smoking for 5 seconds
