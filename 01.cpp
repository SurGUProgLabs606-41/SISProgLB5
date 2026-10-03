#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <string.h>

int main() {
    Display *display = XOpenDisplay(NULL); //Подключение к X-серверу
    if (display == NULL) { //Проверка на успешное подключение
        fprintf(stderr, "Не удалось подключиться к X-серверу\n");
        return 1;
    }

    // Получение информации о экране и корневом окне
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    // Создание окна
    Window window = XCreateSimpleWindow(
        display, root,
        100, 100,
        800, 600,
        1,
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );

    //Называние окна и его параметры
    const char *title = "Лабораторная работа 2";
    Atom utf8        = XInternAtom(display, "UTF8_STRING", False);
    Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", False);

    XChangeProperty(
        display, window,
        net_wm_name, utf8, 8,
        PropModeReplace,
        (const unsigned char*)title,
        (int)strlen(title)
    );

    XStoreName(display, window, title);

    // Отслежуемые события: перерисовка, нажатие клавиш, мыши и изменение структуры окна
    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask | ButtonMotionMask | StructureNotifyMask);

    //Отображение окна на экране
    XMapWindow(display, window);

    //Параметры отрисовки
    Visual   *visual   = DefaultVisual(display, screen);
    Colormap  colormap = DefaultColormap(display, screen);

    //Работа со шрифтами Xft
    XftFont *font = XftFontOpenName(display, screen, "Helvetica-16");

    if (!font)
        font = XftFontOpenName(display, screen, "sans-16");
    if (!font)
        font = XftFontOpenName(display, screen, "Monaco-16");
    if (!font) {
        fprintf(stderr, "Не удалось загрузить шрифт Xft\n");
        XCloseDisplay(display);
        return 1;
    }

    //Создание цвета для текста и его инициализация
    XftColor color;

    XRenderColor rcolor = {0x0000, 0x0000, 0x0000, 0xFFFF}; // чёрный

    XftColorAllocValue(display, visual, colormap, &rcolor, &color);

    //Создание объекта для отрисовки текста основного окна
    XftDraw *draw = XftDrawCreate(display, window, visual, colormap);

    // Текст, который выводится в окне
    const char *text = "Евгений Каличак, группа 606-41";

    // Текст после нажатия Space
    const char *spaceText = "Вы нажали Space!";

    // Текущий текст
    const char *currentText = text;

    // Создание кнопки
    Window button = XCreateSimpleWindow(display, window, 50, 100, 200, 50, 1,
        BlackPixel(display, screen),
        0xDDDDDD
    );

    // Отслеживаем перерисовку и нажатие на кнопку
    XSelectInput(display, button, ExposureMask | ButtonPressMask);

    // Отображение кнопки
    XMapWindow(display, button);

    // Создание отдельного объекта XftDraw для кнопки
    XftDraw *buttonDraw = XftDrawCreate(display, button, visual, colormap);

    // Текст кнопки
    const char *buttonText = "Нажать";

    //Программа постоянно ждёт события от X-сервера и обрабатывает их
    XEvent event;

    while (1) {

        XNextEvent(display, &event);

        // Обработка событий основного окна
        if (event.xany.window == window) {

            switch (event.type) {

                case Expose: //Событие перерисовки окна

                    XftDrawStringUtf8(draw,&color,font,50, 50,
                        (const FcChar8*)currentText,
                        (int)strlen(currentText)
                    );

                    break;

                case KeyPress: { //Событие нажатия клавиши

                    KeySym key = XLookupKeysym(&event.xkey, 0);

                    // Нажатие Escape - выход
                    if (key == XK_Escape) {
                        goto done;
                    }

                    // Нажатие Space - изменение текста
                    if (key == XK_space) {
                    
                    // Переключаем текст
                    if (currentText == text) {
                        currentText = spaceText;
                    }
                    else {
                        currentText = text;
                    }

                    // Очищаем окно
                    XClearWindow(display, window);

                    // Рисуем текущий текст
                    XftDrawStringUtf8(draw,&color,font,50, 50,
                        (const FcChar8*)currentText,
                        (int)strlen(currentText)
                    );

                    XFlush(display);
                }

                    break;
                }

                case ButtonPress: { //Событие нажатия мыши

                    printf(
                        "Клик мыши: X = %d, Y = %d\n",
                        event.xbutton.x,
                        event.xbutton.y
                    );

                    fflush(stdout);

                    break;
                }

                case MotionNotify: //Событие движения мыши

                    // Проверяем, зажата ли левая кнопка мыши
                    if (event.xmotion.state & Button1Mask) {

                        // Рисуем круг вместо одной точки
                        XFillArc(display, window, DefaultGC(display, screen), event.xmotion.x - 4, event.xmotion.y - 4, 8, 8, 0, 360 * 64);

                        XFlush(display);
                    }

                    break;

                case DestroyNotify: //Событие закрытия окна

                    goto done;
            }
        }

        // Обработка событий кнопки
        if (event.xany.window == button) {

            switch (event.type) {

                case Expose: // Перерисовка кнопки

                    // Координаты здесь относительно самой кнопки
                    XftDrawStringUtf8(buttonDraw, &color, font, 65, 32, (const FcChar8*)buttonText, (int)strlen(buttonText));

                    break;

                case ButtonPress: // Нажатие кнопки

                    printf("Кнопка нажата!\n");
                    fflush(stdout);

                    break;
            }
        }
    }

//Завершение работы и освобождение ресурсов
done:

    XftDrawDestroy(buttonDraw); // Освобождение XftDraw кнопки
    XftDrawDestroy(draw);       // Освобождение XftDraw основного окна

    XftColorFree(display, visual, colormap, &color); // Освобождение ресурсов XftColor

    XftFontClose(display, font); // Освобождение ресурсов XftFont

    XDestroyWindow(display, button); // Уничтожение кнопки
    XDestroyWindow(display, window); // Уничтожение окна

    XFlush(display); // Очистка буфера команд X-сервера

    XCloseDisplay(display); // Закрытие соединения с X-сервером

    return 0; // Завершение программы
}