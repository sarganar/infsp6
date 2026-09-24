! INFSP Version
!
! Cortos.h v2.0 biplataforma
!
! Creación de cortometrajes (secuencias no interactivas)
!
! Para documentacion y ejemplo de uso ver cortodem.inf
!

! Las dos lineas siguientes son para que esto siga funcionando con el
! antiguo compilador inform 6.15, que no define la constante
! TARGET_ZCODE.
#ifndef WORDSIZE;
Constant WORDSIZE = 2;
Constant TARGET_ZCODE;
#endif;



!=========================================================
! Variables globales de interés para el usuario
!=========================================================

Global velocidad_texto=20;


!=========================================================
! Variables y funciones auxiliares
!=========================================================

Array StringAux->500;

! Calcula la longitud de la cadena str
[ LStrLen str;
    str.print_to_array(StringAux);
    return StringAux->0;
];



!=========================================================
! FUNCIONES de interés para el usuario
!=========================================================


!++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! Versión de PrintPausa y Erase_Window para la máquina Z 
!
#ifdef TARGET_ZCODE;
! Funcion "tonta" necesaria para una lectura de teclado temporizada
[ Cortos_Volver;
    rtrue;
];

! PrintPausa
!
! La siguiente función imprime "msg" y despues espera
! "delay" decimas de segundo
[ PrintPausa msg delay k;
    if (msg~=0) print (string) msg;
    if (delay==0)
        @read_char 1 0 0 k;
    else @read_char 1 delay Cortos_Volver k;
    return k;
];

! Erase_Window
!
! Borrar la pantalla
[ Erase_Window;
    @erase_window -1;
];
#ifnot;

!++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! Versión  de PrintPausa y Erase_Window para la máquina Glulx
!
! PrintPausa
!
! La siguiente función imprime "msg" y despues espera
! "delay" decimas de segundo
[ PrintPausa s delay;
    if (s) print (string) s;
!    print "^[Esperando ", delay, " décimas de segundo]^";
    glk($00D2, gg_mainwin); ! glk_request_char_event(gg_mainwin);
    glk($00D6, delay*100); ! request_timer_events
    while(1) 
    {
    glk($00C0, gg_arguments); ! glk_select(gg_arguments);
    if ((gg_arguments-->0) == 2) break; ! 2=evType_CharInput
    if ((gg_arguments-->0) == 1) ! evType_Timer
    {
            glk($00D3, gg_mainwin); ! cancel_char_event
                glk($00D6, 0); ! request_timer_events (cancelados)
!            print "[Timeout]^";
            return 0;
    }
    }
    glk($00D6, 0); ! request_timer_events (cancelados)
    return gg_arguments-->2;
];

! Erase_Window
! Borra la pantalla y cierra la ventana de citas (boxes)
[ Erase_Window;
    if (gg_quotewin) ! Cerrar la ventana de boxes
    {
        glk($0024, gg_quotewin, 0); ! close_window
        gg_quotewin = 0;
    }
    glk($002A, gg_mainwin);  ! window_clear
    if (gg_statuswin) glk($002A, gg_statuswin); ! window_clear
];
#endif;
!
!++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


! PrintAutoPausa
!
! La siguiente función imprime txt, y despues espera
! un tiempo que depende de la longitud del texto
! y de la variable global velocidad_texto
! La espera termina tambien si el lector pulsa una tecla
[ PrintAutoPausa txt k;
  if (velocidad_texto==0)
    k=PrintPausa(txt,0);
  else
    k=PrintPausa(txt, LStrLen(txt)*10/velocidad_texto+10);
  return k;
];      




!=======================================================
! CLASE Cortometraje
!=======================================================

class     CortoMetraje
 with      proyeccion [i t;
        if (~~i) Erase_Window();
        if (self provides description) {
            for (i=0:i<(self.#description)/WORDSIZE:i++) {
            ! Modificado para poner texto de corrido
            ! si velocidad_texto vale 0 (cero)
            if ((self.&description)-->i ofclass string)
            {
                if (velocidad_texto==0)
                    print (string) (self.&description)-->i;
                else
                    t=PrintAutoPausa((self.&description)-->i);
            }
            ! Código original, antes de la modificación
            ! t=PrintAutoPausa((self.&description)-->i);
            else if ((self.&description)-->i ofclass routine)
            {
                indirect((self.&description)-->i);
            t=PrintPausa(0, self.delay);
            }
            else jump error; 
            if (t=='q' or 'Q') print_ret " ";
            }
        return;
        }
        .error;
        
        #ifdef DEBUG;
        print "Error: CortoMetraje sin descripción.^";
        #endif;
    ],
    delay 0,
 has proper;


