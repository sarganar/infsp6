! ZIPI v.2.0     (c) 2001 Zak McKraken
! ================================================================
!
! Z-InvisiPistas Interactivas
!
! Esta librería forma parte de la utilidad ZIPI que sirve para crear
! ficheros de InvisiPistas Interactivas a partir de un fichero de
! texto escrito en un lenguaje específico ZTML (Zipi Text Markup
! Lenguage).
!
! Esta versión de ZIPI es biplataforma (puede generar ficheros
! legibles bajo Glulx, si el compilador recibe el flag -G)  
!
!Switches d2;
System_file;

! Compatibilidad con inform 6.15
#ifndef WORDSIZE;
Constant WORDSIZE = 2;
Constant TARGET_ZCODE;
#endif;

!================================================================
! Clases
!
Class ZIPI_Menu
    with ZIPI_titulo, ZIPI_item;

Class ZIPI_Pista
    with ZIPI_titulo, ZIPI_pistas;

Class ZIPI_Otro
    with ZIPI_titulo, ZIPI_ejecutar;

!================================================================
! Variables globales
!
Array  ZIPI_buffer    string 120;         ! Buffer para interpretar la
                                          ! línea de entrada principal
Array  ZIPI_parse     string 64;          ! Tabla de parsing correspondiente
Global ZIPI_modo;
#ifdef TARGET_GLULX;
#ifndef gg_statuswin;
Global gg_statuswin;
Array gg_event --> 4;
Array gg_arguments --> 8;
#endif;
#endif;
Constant ZIPI_EVENTO_HYPER 400;

!================================================================
! Objetos principales
!
ZIPI_Menu Menu_vacio;
ZIPI_Pista Pista_vacia;
ZIPI_Otro Otro_vacio;

!================================================================
! Rutinas de bajo nivel (biplataforma)
!
! Inicializar Glk (vacia para el caso Z)
!________________________________________________________________
[ ZIPI_InitGlk;
#ifdef TARGET_GLULX;
!  @setiosys 2 0;
!  gg_statuswin=glk($23, 0,0,0,4,211);
!  glk($2F, gg_statuswin);

! Cerramos la ventana de gráficos ...

#endif;
];


! Borrar pantalla  
!________________________________________________________________
[ ZIPI_clrscr;
#ifdef TARGET_GLULX;
   glk(42,gg_statuswin);
#ifnot;
   @erase_window -1;
#endif;
];

! Leer un caracter del teclado
!________________________________________________________________
#ifdef TARGET_GLULX;
[ ZIPI_tecla ;
    glk($00D2, gg_statuswin); ! glk_request_char_event(gg_statuswin);
    glk(258, gg_statuswin); ! glk_request_hyperlink_event
    while(1) 
    {
        glk($00C0, gg_arguments); ! glk_select(gg_arguments);
        if ((gg_arguments-->0) == 2) { ! 2=evType_CharInput
            glk(259, gg_statuswin); ! glk_cancel_hyperlink_event
            return gg_arguments-->2;
    }
        if ((gg_arguments-->0) == 8) {  ! evType_Hyperlink
            glk(211, gg_statuswin); ! glk_cancel_char_event
        print "^^YAY^^";
            return gg_arguments-->2 + ZIPI_EVENTO_HYPER;
    }
    }
];
#ifnot;
[ ZIPI_tecla tecla;
  @read_char 1 -> tecla;
  return tecla;
];
#endif;

! Mover el cursor
!________________________________________________________________
[ ZIPI_setcursor x y;
#ifdef TARGET_GLULX;
 glk($002B, gg_statuswin, x-1, y-1); ! window_move_cursor
#ifnot;
 @set_cursor y x;
#endif;
];

! Rellenar con n espacios
!________________________________________________________________
[ ZIPI_spaces n i;
#ifdef TARGET_GLULX;
 for (i=0:i<n:i++) print (char) ' ';
#ifnot;
 spaces n;
 i=i;      ! eliminar warning
#endif;
];

! Seleccionar la ventana textgrid
!________________________________________________________________
[ ZIPI_setwindow;
#ifdef TARGET_GLULX;
  if (gg_statuswin==0) return;
  glk($2F, gg_statuswin);
#ifnot;
  @set_window 1;
#endif;
];

! Averiguar el ancho de esa ventana
!________________________________________________________________
[ ZIPI_getwidth;
#ifdef TARGET_GLULX;
    glk($0025, gg_statuswin, gg_arguments, gg_arguments+4); ! window_get_size
    return  gg_arguments-->0;
#ifnot;
    return 0->33;
#endif;
];

! Pinta un titulo (video inverso en la maquina Z) en la linea superior
!________________________________________________________________
[ ZIPI_PintaTitulo titulo j;
    ZIPI_clrscr();
#ifdef TARGET_ZCODE;
    @split_window 1;
#endif;
    ZIPI_setwindow();
    ZIPI_setcursor(1, 1);
!    ZIPI_reverse();
    j = ZIPI_getwidth();
    if (j == 0) j = 80;
    ZIPI_spaces(j);
    ZIPI_setcursor(2,1);
#ifdef TARGET_GLULX;
    glk(256, 1);     ! hiperenlace 1    
#endif;
    print (string)titulo;
#ifdef TARGET_GLULX;
    glk(256, 0);     ! hiperenlace 0 (fin del tag)    
#endif;
    ZIPI_roman();
];

! Estilos de texto
!________________________________________________________________
[ ZIPI_bold;
#ifdef TARGET_GLULX;
    glk($0086, 4); ! set subheader style
#ifnot;
   style bold;
#endif;
];

[ ZIPI_reverse;
#ifdef TARGET_GLULX;
    glk($86, 3);
#ifnot;
    style reverse;
#endif;
];

[ ZIPI_roman;
#ifdef TARGET_GLULX;
        glk($0086, 0); ! set normal style
#ifnot;
  style roman;
#endif;
];

!================================================================
! Rutinas de alto nivel (independientes de la plataforma
!
[ ZIPI_RunMenu _m   i j count cur key target redibujar;

    redibujar = 1;

    cur = 0;
    count = _m.#ZIPI_item / WORDSIZE;

    
    for(::) {
        
        if (redibujar) {
            ZIPI_PintaTitulo(_m.ZIPI_titulo);
            redibujar = 0;
        }

        for(i = 0: i < count: i++) {
            j = i + 3;
            ZIPI_setcursor(5,j);
            print (string)(_m.&ZIPI_item-->i).ZIPI_titulo;
        }
            j = i + 3;
            ZIPI_setcursor(5,j);
            print "Volver";
        
                
        j = cur + 3;
        ZIPI_setcursor(2,j);
        print ">";
        ZIPI_setcursor(2, j);
        key=ZIPI_tecla();
        print " ";
              if (key>ZIPI_EVENTO_HYPER) {
          cur = key-ZIPI_EVENTO_HYPER - 2;
         if (cur==-1) key=27;
         else key=13;
        }         
        switch(key) {
            'k', 'p', '-', '_', 129, -4:
                cur--;
                if (cur < 0) cur = count-1;
                break;
            'j', 'n', '=', '+', 130, -5:
                cur++;
                if (cur > count) cur = 0;
                break;
            'q', 'Q', 27, 131, 10, 8, -2:
                return;

            132, 13, 'n', ' ', -3, -6:
            
                if (cur==count) return;
                target = _m.&ZIPI_item-->cur;
                if (target ofclass ZIPI_Menu)
                    ZIPI_RunMenu(target);
                else if (target ofclass ZIPI_Pista)
                    ZIPI_RunPista(target);
                else
                    ZIPI_RunOtro(target);

                redibujar = 1;
                break;
        }
    }
];



[ ZIPI_RunPista _h   count cur key done;
    
    count = _h.#ZIPI_pistas / WORDSIZE;
    cur = 0;
    done = 0;
#ifdef TARGET_GLULX;    
    ZIPI_cambiaventana();
    print (string) _h.ZIPI_titulo, "^^";
#endif;   
    
#ifdef TARGET_ZCODE;    
     ZIPI_PintaTitulo(_h.ZIPI_titulo);
    @set_window 0;
#endif;
    print "^[Pulsa 'P' para una pista (", count, " en total)";
    print ", o 'Q' para parar]^^";
    
    do {
    key=ZIPI_tecla();
    switch(key) {
     'p', 'P':
        cur++;
        print "(", cur, "/", count, ") ",
        (string)_h.&ZIPI_pistas-->(cur-1), "^^";
        break;
     'q', 'Q', 27, 131, 10, 8:
        return;
    }
    
    } until(cur >= count);
    ZIPI_Espera();
#ifdef TARGET_GLULX;    
    ZIPI_cambiaventana_texto();
#endif;    
];

[ ZIPI_RunOtro _o;
#ifdef TARGET_GLULX;
    ZIPI_cambiaventana();
    ZIPI_Bold();
    print (string)_o.ZIPI_titulo, "^^";
    ZIPI_Roman();
#endif;    
!    ZIPI_PintaTitulo(_o.ZIPI_titulo);

#ifdef TARGET_ZCODE;
    ZIPI_PintaTitulo(_o.ZIPI_titulo);
    @set_window 0;
#endif;
    print "^";
    _o.ZIPI_ejecutar();
    ZIPI_Espera();
#ifdef TARGET_GLULX;    
    ZIPI_cambiaventana_texto();
#endif;    
];

[ ZIPI_Center s    len wid off;
    wid = ZIPI_getwidth();
    if (wid == 0) wid = 80;
    off = (wid - len) / 2;
    ZIPI_spaces(off);
    print (string)s, "^";
];



! Menu en modo texto, para usuarios con terminales pequeñitas
! como la de la Palm Pilot
#ifdef TARGET_ZCODE;
! Tomada de eparserm.h, libreria InformatE
[ ZIPI_IntentarNumero i j c num len mul tot d digit;

   i=1*4+1; j=ZIPI_parse->i; num=j+ZIPI_buffer; len=ZIPI_parse->(i-1);

   if (len>=4) mul=1000;
   if (len==3) mul=100;
   if (len==2) mul=10;
   if (len==1) mul=1;

   tot=0; c=0; len=len-1;

   for (c=0:c<=len:c++)
   {   digit=num->c;
       if (digit=='0') { d=0; jump digok; }
       if (digit=='1') { d=1; jump digok; }
       if (digit=='2') { d=2; jump digok; }
       if (digit=='3') { d=3; jump digok; }
       if (digit=='4') { d=4; jump digok; }
       if (digit=='5') { d=5; jump digok; }
       if (digit=='6') { d=6; jump digok; }
       if (digit=='7') { d=7; jump digok; }
       if (digit=='8') { d=8; jump digok; }
       if (digit=='9') { d=9; jump digok; }
       return -1000;
     .digok;
       tot=tot+mul*d; mul=mul/10;
   }
   if (len>3) tot=10000;
   return tot;
];

[ ZIPI_RunMenuT _m cur count i j target;

    cur = 0;
    count = _m.#ZIPI_item / WORDSIZE;
    
    for(::) {
    ZIPI_clrscr();
        ZIPI_bold();
    print (string) _m.ZIPI_titulo, "^^";
         ZIPI_roman();
    for(i = 0: i < count: i++) {
        print i+1, ": ", (string)(_m.&ZIPI_item-->i).ZIPI_titulo, "^";
    }
    print "^Elige opción (0=salir) > ";
    read ZIPI_buffer ZIPI_parse;
    j=ZIPI_IntentarNumero();
    if (j==-1000) 
    {
        print "Debes escribir un numero^";
        ZIPI_Espera();
    }
    else if (j>count)
    {
        print "El numero va de 1 a ", count, " (0=salir)";
        ZIPI_Espera();
    }
    else 
    {
        if (j==0) return;
        target = _m.&ZIPI_item-->(j-1);
        if (target ofclass ZIPI_Menu)
        ZIPI_RunMenuT(target);
        else if (target ofclass ZIPI_Pista)
        ZIPI_RunPistaT(target);
        else
        ZIPI_RunOtroT(target);
    }
    }
];

[ ZIPI_RunPistaT _h   count cur key done;

    count = _h.#ZIPI_pistas / 2;
    cur = 0;
    done = 0;
    
    @erase_window -1;
    style bold;
    print (string) _h.ZIPI_titulo, "^^";
    style roman;
    
    print "^[Pulsa 'P' para una pista (", count, " en total)";
    print ", o 'Q' o '0' para parar]^^";
    
    do {
    @read_char 1 -> key;
    switch(key) {
     'p', 'P':
        cur++;
        print "(", cur, "/", count, ") ",
        (string)_h.&ZIPI_pistas-->(cur-1), "^^";
        break;
     'q', 'Q', 27, 131, 10, 8, '0':
        return;
    }
    
    } until(cur >= count);
    ZIPI_Espera();
];

[ ZIPI_RunOtroT _o;
    style bold;
    print (string) _o.ZIPI_titulo;
    style roman;
    print "^";
    _o.ZIPI_ejecutar();
    ZIPI_Espera();
];
#endif;

! La otra version para Glulx no usa numeros para navegar por los
! menus, sino hiperenlaces.
!
#ifdef TARGET_GLULX;
[ ZIPI_cambiaventana;
    glk(36, gg_statuswin, gg_event);   ! cerrarla
    gg_statuswin=glk($23, 0,0,0,3,211); ! Reabrirla tipo buffertext
    glk($2F, gg_statuswin);
];

[ ZIPI_cambiaventana_texto;
    glk(36, gg_statuswin, gg_event);   ! cerrarla
    gg_statuswin=glk($23, 0,0,0,4,211); ! Reabrirla tipo texto ...
    glk($2F, gg_statuswin);
];
    

[ ZIPI_leerEventoHyper ;
    glk(258, gg_statuswin); ! glk_request_hyperlink_event
    while(1) 
    {
        glk($00C0, gg_arguments); ! glk_select(gg_arguments);
        if ((gg_arguments-->0) == 8) {  ! evType_Hyperlink
            return gg_arguments-->2;
    }
    }
];

[ ZIPI_RunMenuT _m cur count i j target;

    cur = 0;
    count = _m.#ZIPI_item / WORDSIZE;
    
    for(::) {
        ZIPI_cambiaventana();
    ZIPI_clrscr();
        ZIPI_bold();
    print (string) _m.ZIPI_titulo, "^^";
         ZIPI_roman();
    for(i = 0: i < count: i++) {
            print "    ";
        glk(256, i+1);
        print (string)(_m.&ZIPI_item-->i).ZIPI_titulo,"^";
            glk(256,0);
    }
    print "^Pincha con el ratón sobre una de las anteriores ";
        glk(256, 100);
        print "(o aqui para salir)";
        glk(256,0);
    print "^^";
    j=ZIPI_LeerEventoHyper();
    if (j==100) return ZIPI_clrscr();
    target = _m.&ZIPI_item-->(j-1);
    if (target ofclass ZIPI_Menu)
      ZIPI_RunMenuT(target);
    else if (target ofclass ZIPI_Pista)
       ZIPI_RunPistaT(target);
    else
       ZIPI_RunOtroT(target);
    }
];

[ ZIPI_RunPistaT _h   count cur key done;

    count = _h.#ZIPI_pistas / WORDSIZE;
    cur = 0;
    done = 0;
    
    ZIPI_cambiaventana();
    ZIPI_clrscr();
    ZIPI_bold();
    print (string) _h.ZIPI_titulo, "^^";
    ZIPI_roman();
    
    print "^";
    glk(256, 1);  print "Pulsa aqui para una pista";   glk(256,0);
    print " (hay ", count, " en total) o ";
    glk(256,2);  print " aqui para retornar.^";  glk(256,0);

    do {
    key=ZIPI_leerEventoHyper();
    switch(key) {
     1:
        cur++;
            print "^^", (string)_h.&ZIPI_pistas-->(cur-1), "^";
        print "  ";
            if (count-cur > 1) {
          glk(256,1); print "¿Otra pista?"; glk(256,0);
              print " (quedan ", count-cur, ") ¿O ";
              glk(256,2); print " ya es suficiente?"; glk(256,0);
            }
            else if (count-cur > 0) {
          glk(256,1); print "¿La última pista?"; glk(256,0);
              print " ¿O prefieres ";
              glk(256,2); print "pensartelo?"; glk(256,0);
        }     
        break;
     2:
        return;
    }
    
    } until(cur >= count);
    ZIPI_Espera();
    ZIPI_cambiaventana_texto();
];

[ ZIPI_RunOtroT _o;
    ZIPI_cambiaventana();
    ZIPI_bold();
    print (string) _o.ZIPI_titulo;
    ZIPI_roman();
    print "^";
    _o.ZIPI_ejecutar();
    ZIPI_Espera();
    ZIPI_cambiaventana_texto();
];
#endif;


[ ZIPI_Espera;
    print "^^[Pulsa una tecla para seguir]";
    ZIPI_tecla();
];
