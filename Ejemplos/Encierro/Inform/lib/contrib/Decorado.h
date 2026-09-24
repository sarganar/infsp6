! INFSP Version

! Decorados.h (c) 2000 Zak McKracken
! V2.0 biplataforma
!
!
!  Se define la clase decorado.
!
!  Los objetos de esta clase son en realidad varios objetos del juego,
!  recogidos en un solo objeto del codigo.
!
!  Se entiende que son objetos minimalistas, con los que apenas se
!  puede interactuar, salvo examinarlos.
!
!  Un objeto de la clase decorado, no puede tener short_name (nombre_corto) ni
!  description, ya que en realidad representa a muchos objetos
!  diferentes. Lo que tiene en cambio es la propiedad 'describir', que
!  contiene una lista con la siguiente estructura:
!
!  with describir
!    'palabra' "descripcion" GENERO
!    'palabra2' "descripcion2" GENERO2
!    etc.
!    ,
!
! Ejemplo:
!
! Object decorado_celda Celda
! class decorado
! with  describir
!        'pared' "Paredes lugubres y frias" G_FEMENINO
!        'paredes' "Paredes lugubres y frias" G_FEMENINO+G_PLURAL
! ;
!
!
!  Observa que los elementos de la lista no llevan comas separando. Ni
!  siquiera al final de cada linea. Solo al final de todos los
!  elementos, habra que poner una coma (si vas a seguir definiendo
!  propiedades o atributos), o directamente punto y coma (si has
!  terminado la definicion del objeto).
!
! Como funciona:
!
!  Cuando el jugador intente cualquier accion usando como sustantivo
!  'pared', por ejemplo, y el objeto decorado_celda esta a su alcance
!  (el jugador esta dentro de la celda), este objeto se dara por
!  referenciado, y asi se lo notificara al parser, actualizando ademas
!   los siguientes campos del objeto decorado_celda:
!
!   number (cantidad)  -> Toma el valor de la palabra usada por el jugador
!                         ('pared')
!   description        -> Toma el valor de la cadena que sigue a 'pared', en
!                          la propiedad describir es decir "Paredes lugubres y
!                          frias"
!   gender (genero)    -> Toma el genero asociado con esa entrada, es decir
!                         G_FEMENINO
!
!  Despues deja que el parser continue. La libreria, poco despues,
!  llamara al objeto  decorado_celda, a su rutina before (antes), para
!  informarle de la accion que se pretende hacer con él. La rutina
!  proporcionada en esta clase, por defecto "deja pasar" la
!  accion Examine, pero impide cualquier otra con el mensaje "Dejalo,
!  solo es decorado".
!
!  Si la accion es Examine, la libreria llevara a cabo su accion
!  estandar, y por tanto poco despues imprimira la descripcion de ese
!  objeto, que toma del campo description y que ya contiene la cadena
!  correcta.
!
!  Puedes sobreescribir la rutina before en los objetos que derive de
!  esta clase, y emitir mensajes genericos a tu gusto, del
!  estilo de:
!
!    "Deja ", (el) self, " en paz. No ", (es) self, " mas que decorado.";
!
!  Y el nombre que saldra sera en cada caso el de la palabra que haya
!  usado el jugador, por ejemplo:
!
!  > EMPUJA PAREDES
!  Deja las paredes en paz, no son mas que decorado.
!
!  Incluso, si quisieras particularizar para un objeto concreto,
!  podrias examinar la propiedad number, que te dira qué palabra ha
!  usado el jugador. Por ejemplo:
!
!  Empujar: if (self.number=='techo') "No llegas.",
!           "Deja ", (el) self, " en paz.";
!
!
!  LIMITACIONES: si el nombre corto tiene mas de nueve letras, cuando
!  sea impreso sera truncado, por ejemplo:
!  > EMPUJA INSTALACION
!  Deja la instalaci en paz.
!
!  Puede evitarse redefiniendo habilmente la rutina short_name en el
!  objeto derivado de la clase, y cambiandolo por algo como:
!
!     if (self.number=='instalacion') print "instalacion";
!     else print (address) self.number;
!     rtrue;
!
!  Pero evidentemente habria que particularizarlo para cada caso.
!

! Para compatibilidad con Inform 6.15 InformATE 6/7
#ifndef WORDSIZE;
Constant WORDSIZE=2;
Constant TARGET_ZCODE;
#endif;


System_file;

Message "Incluyendo modulo de objetos de decorados, por Zak";

!
! Rutina que ejecuta rutinas si las encuentra
!
! Ifndef VR;
[ VR valor;

    ! print "[VR: ", valor, " --> ", ZRegion( valor ), " ]^";
    if ( ZRegion( valor ) == 2 )
        return valor();
    else
        return valor;
];
! EndIf;

!
! CLASE DECORADO
!
class Decorado
 with    description 0,
    number 0,
    describe 0,
    gender 0,
    short_name [;
        print (address) self.number;
        rtrue;
    ],
      parse_name [i n w;
        self.description=0;
        w=NextWord();
        if (w=='el' or 'la' or 'los' or 'las')
        w=NextWord();
        n=(self.#describe)/(3*WORDSIZE);
        ! print n, ": ";
        for (i=0:i<n:i=i+1) 
        {
            if ((self.&describe)-->(i*3)==w) 
            {
                ! print i, " <", (address)(self.&describir)-->(i*3+1), "> ";
                self.description=VR((self.&describe)-->(i*3+1));
                self.number=w;
                self.gender = (self.&describe)-->(i*3+2);
                return 1;
            }
        }
    ],
    before [;
     Examine: rfalse;
     default: "Déjalo, solo es decorado.";
    ],
 has scenery concealed;

