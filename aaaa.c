#ifndef LIB_CLIENT_H_
#define LIB_CLIENT_H_

#define NO_KEY_PRESSED '\0'

#define NO_MESSAGE -1
#define SERVER_DISCONNECTED -2
#define WAIT_FOR_IT 1
#define DONT_WAIT 2

enum conn_ret_t {
  SERVER_UP,
  SERVER_DOWN,
  SERVER_FULL,
  SERVER_CLOSED,
  SERVER_TIMEOUT
};

void closeConnection();
enum conn_ret_t connectToServer(const char *server_IP);
int sendMsgToServer(void *msg, int size);
int recvMsgFromServer(void *msg, int option);
char getch();

#endif // LIB_CLIENT_H_


#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CLIENTSTART 41
#define STARTGAME 6
#define ATAQUE 5
#define CIMA 1
#define BAIXO 2
#define DIREITA 3
#define ESQUERDA 4
#define TICKTIMER 31
#define UPCIDADE 11
#define UPQUARTEL 12
#define UPMINA 13
#define UPMURALHA 14
#define DESCONECTADO 50

float width=1440.0;
float height=810.0;

typedef struct{
    int quartel;
    int cidade;
    int mina;
    int muralha;
}upgrades;

typedef struct{
    float vida;
    float vidaM;
    float dps;
}castelo;

typedef struct{
    float x,y;
    char nome[20];
    int personagem;
    float health;
    float dinheiro;
    castelo castelos[3];
    float ataque;
    int healthM;
    int basex;
    int basey;
    int persoX;
    int persoY; 
    int respawn;
    upgrades up;
    int flip;
}player;

typedef struct{
    player jogadores[4];
    int flag;
}mensagem;

enum conn_ret_t tryConnect(char IP[]) {
  return connectToServer(IP);
}
void assertConnection(char IP[],bool *chat,char *nome,int personagemJogador) {
    puts("aaaa");
  enum conn_ret_t ans = tryConnect(IP);
    if(ans != SERVER_UP){
        puts("entrou nessa porra");
        *chat=false;
    }
    else{
        int len = (int)strlen(nome);
        sendMsgToServer(nome, len + 1);
        sendMsgToServer(&personagemJogador,sizeof(int));
    } 
}

void cameraUpdate(int *cameraPosition,float x, float y, int wiplayer, int heiplayer){
    cameraPosition[0]=-((int)width/2)+(((int)x*64)+(wiplayer/2));
    cameraPosition[1]=-((int)height/2)+(((int)y*64)+(heiplayer/2));

    if(cameraPosition[0]<0){
        cameraPosition[0]=0;
    }
    if(cameraPosition[1]<0){
        cameraPosition[1]=0;
    }
}

void preenchePersonagens(ALLEGRO_BITMAP ***personagens){
    (*personagens)=(ALLEGRO_BITMAP**) calloc(3,sizeof(ALLEGRO_BITMAP*));

    (*personagens)[0]=al_load_bitmap("pers1.png");
    (*personagens)[1]=al_load_bitmap("pers2.png");
    (*personagens)[2]=al_load_bitmap("pers3.png");
}

void preencheUpgrades(ALLEGRO_BITMAP ****upgradeBits){
    int i;
    (*upgradeBits)=(ALLEGRO_BITMAP***) calloc(4,sizeof(ALLEGRO_BITMAP**));
    for(i=0;i<4;i++){
        (*upgradeBits)[i]=(ALLEGRO_BITMAP**) calloc(3,sizeof(ALLEGRO_BITMAP*));
    }

    (*upgradeBits)[0][0]=al_load_bitmap("cidade2.png");
    (*upgradeBits)[0][1]=al_load_bitmap("cidade3.png");
    (*upgradeBits)[0][2]=al_load_bitmap("cidade4.png");
    (*upgradeBits)[1][0]=al_load_bitmap("quartel1.png");
    (*upgradeBits)[1][1]=al_load_bitmap("quartel2.png");
    (*upgradeBits)[1][2]=al_load_bitmap("quartel3.png");
    (*upgradeBits)[2][0]=al_load_bitmap("mina1.png");
    (*upgradeBits)[2][1]=al_load_bitmap("mina2.png");
    (*upgradeBits)[2][2]=al_load_bitmap("mina3.png");
    (*upgradeBits)[3][0]=al_load_bitmap("muralha1.png");
    (*upgradeBits)[3][1]=al_load_bitmap("muralha2.png");
    (*upgradeBits)[3][2]=al_load_bitmap("muralha3.png");
}



int main(void)
{
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *queue,*queueTutorial,*queueLogin,*queueJogo,*queueInventario;
    ALLEGRO_BITMAP *cursor,*fundo,**personagens,*bau,***upgradeBits,*mapa;
    ALLEGRO_TIMER *timer;
    ALLEGRO_FONT *fonte,*fontemaior,*fontejogar,*fontesair,*fontetutorial,*fontetitulo,*fontemenor,*fonteupgrades;
    ALLEGRO_TRANSFORM camera;

    al_init();
    al_init_image_addon();
    al_init_primitives_addon();
    al_install_keyboard();
    al_install_mouse();
    al_init_font_addon();
    al_init_ttf_addon();

    
    preenchePersonagens(&personagens);
    preencheUpgrades(&upgradeBits);
    display = al_create_display(1440, 810);
    queue=al_create_event_queue();
    timer=al_create_timer(1.0/15);
    fonte=al_load_font("Enchanted Land.otf",45,0);
    fontemaior=al_load_font("Enchanted Land.otf",60,0);
    fontetitulo=al_load_font("Enchanted Land.otf",80,0);
    fontemenor=al_load_font("Enchanted Land.otf",30,0);
    fonteupgrades=al_load_font("Enchanted Land.otf",20,0);

    al_register_event_source(queue,al_get_mouse_event_source());
    al_register_event_source(queue,al_get_display_event_source(display));
    al_register_event_source(queue,al_get_keyboard_event_source());
    al_register_event_source(queue,al_get_timer_event_source(timer));
    cursor=al_load_bitmap("spellcast.png");
    fundo=al_load_bitmap("wallpaper1.jpg");
    bau=al_load_bitmap("bau.png");
    mapa=al_load_bitmap("mapa.png");
    al_hide_mouse_cursor(display);
    float x=0,y=0;
    bool run=true;
    bool tutorial=false;
    bool login=false;
    player jogador[4];
    player jogadoraux;
    char nomeJogador[20];
    int nomelen=0;
    int personagemJogador;
    int flag;
    int msg=0;
    int id=DESCONECTADO;
    char ip[20];
    int iplen=0;
    int status;
    int i,k;
    char isso[100];
    mensagem mServer;
    al_start_timer(timer);
    al_clear_to_color(al_map_rgb(0,0,0));
    
    while(run){

        ALLEGRO_EVENT event;
        fontetutorial=fonte;
        fontejogar=fonte;
        fontesair=fonte;
        
        al_wait_for_event(queue,&event);
        if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
            run=false;
        }

        if(event.keyboard.keycode==ALLEGRO_KEY_ESCAPE){
            run=false;
        }

        if(event.type==ALLEGRO_EVENT_MOUSE_AXES){
            x=(float)event.mouse.x;
            y=(float)event.mouse.y;
        }
        //jogar
        if(x>=660&&x<=770&&y>=300&&y<=350){
            fontejogar=fontemaior;
            if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                al_destroy_event_queue(queue);
                queueLogin=al_create_event_queue();
                al_register_event_source(queueLogin,al_get_mouse_event_source());
                al_register_event_source(queueLogin,al_get_display_event_source(display));
                al_register_event_source(queueLogin,al_get_keyboard_event_source());
                al_register_event_source(queueLogin,al_get_timer_event_source(timer));
                bool digitando=false;
                login=true;
                //login
                while(login){
                    int sizep[3]={90,90,90};
                    al_wait_for_event(queueLogin,&event);
                    if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
                        run=false;
                        login=false;
                    }

                    if(event.keyboard.keycode==ALLEGRO_KEY_ESCAPE){
                        login=false;
                    }

                    if(event.type==ALLEGRO_EVENT_MOUSE_AXES){
                        x=(float)event.mouse.x;
                        y=(float)event.mouse.y;
                    }
                    
                    //escolhe o personagem 1
                    if(x>=(width/4)-45&&x<=(width/4)+45&&y>=350&&y<=440){
                        sizep[0]=120;
                        if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                            personagemJogador=0;
                        }
                    }
                    //escolhe o personagem 2
                    if(x>=(width/2)-45&&x<=(width/2)+45&&y>=350&&y<=440){
                        sizep[1]=120;
                        if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                            personagemJogador=1;
                        }
                    }
                    //escolhe o personagem 3
                    if(x>=(width*3/4)-45&&x<=(width*3/4)+45&&y>=350&&y<=440){
                        sizep[2]=120;
                        if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                            personagemJogador=2;
                        }
                    }


                    //digitar
                    if(x>=(width/2)-155&&x<=(width/2)+165&&y>=190&&y<=230){
                        if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                            digitando=true;
                            while(digitando){
                                al_wait_for_event(queueLogin,&event);

                                if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
                                    digitando=false;
                                    login=false;
                                    run=false;
                                }

                                if(event.type==ALLEGRO_EVENT_MOUSE_AXES){
                                    x=(float)event.mouse.x;
                                    y=(float)event.mouse.y;
                                }

                                if((event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP&&(x>=(width/2)-155&&x<=(width/2)+165&&y>=190&&y<=230))||event.keyboard.keycode==ALLEGRO_KEY_ENTER){
                                    digitando=false;
                                }

                                //preenche o nome do jogador
                                if(event.type==ALLEGRO_EVENT_KEY_CHAR){
                                    if(event.keyboard.keycode!=ALLEGRO_KEY_BACKSPACE){
                                        nomeJogador[nomelen]=(char)event.keyboard.unichar;
                                        nomelen++;
                                        nomeJogador[nomelen]='\0';
                                    }
                                    else{
                                        if(nomelen!=0){
                                            nomelen--;
                                        }
                                        nomeJogador[nomelen]='\0';
                                    }
                                }
                                if(event.type==ALLEGRO_EVENT_TIMER){
                                    al_draw_bitmap(fundo,0,0,0);
                                    al_draw_text(fontetitulo,al_map_rgb(150,150,150),(width/2)-(float) al_get_text_width(fontetitulo,"Digite seu nome")/2,100,0,"Digite seu nome");
                                    al_draw_text(fontetitulo,al_map_rgb(255,0,0),(width/2)-(float) al_get_text_width(fontetitulo,"Jogar")/2,600,0,"Jogar");
                                    al_draw_text(fontetitulo,al_map_rgb(255,0,0),(width/2)-(float) al_get_text_width(fontetitulo,"Escolha seu personagem")/2,250,0,"Escolha seu personagem");
                                    al_draw_scaled_bitmap(personagens[0],0,0,32,32,(width/4)-((float)sizep[0]/2),350,(float)sizep[0],(float)sizep[0],0);
                                    al_draw_scaled_bitmap(personagens[1],0,0,32,32,(width/2)-((float)sizep[1]/2),350,(float)sizep[1],(float)sizep[1],0);
                                    al_draw_scaled_bitmap(personagens[2],0,0,32,32,(width*3/4)-((float)sizep[2]/2),350,(float)sizep[2],(float)sizep[2],0);
                                    al_draw_filled_rectangle((width/2)-32,700,(width/2)+32,764,al_map_rgb(200,100,100));
                                    al_draw_filled_rectangle((width/2)-155,190,(width/2)+165,230,al_map_rgb(100,100,100));
                                    al_draw_textf(fonte,al_map_rgb(255,0,255),(width/2)-155,190,0,"%s",nomeJogador);
                                    al_draw_bitmap(cursor,x,y,0);
                                    al_flip_display();
                                }
                            }
                        }
                    }
                    //tela de conexao
                    if(x>=(width/2)-32&&x<=(width/2)+32&&y>=700&&y<=764){
                        if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                            bool jogo=false;
                            bool chat=true;
                            flag=60;
                            while(chat){
                                al_wait_for_event(queueLogin,&event);

                                if(event.type==ALLEGRO_EVENT_MOUSE_AXES){
                                    x=(float)event.mouse.x;
                                    y=(float)event.mouse.y;
                                }

                                if(event.keyboard.keycode==ALLEGRO_KEY_ESCAPE){
                                    chat=false;
                                }

                                if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
                                    run=false;
                                    login=false;
                                    chat=false;
                                }

                                //digitando ip
                                if(x>=(width/2)-155&&x<=(width/2)+165&&y>=190&&y<=230){
                                    if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                                        digitando=true;
                                        while(digitando){
                                            al_wait_for_event(queueLogin,&event);

                                            if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
                                                digitando=false;
                                                login=false;
                                                run=false;
                                            }

                                            if(event.type==ALLEGRO_EVENT_MOUSE_AXES){
                                                x=(float)event.mouse.x;
                                                y=(float)event.mouse.y;
                                            }

                                            if((event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP&&(x>=(width/2)-155&&x<=(width/2)+165&&y>=190&&y<=230))||event.keyboard.keycode==ALLEGRO_KEY_ENTER){
                                                assertConnection(ip,&chat,nomeJogador,personagemJogador);
                                                i=recvMsgFromServer(&id,WAIT_FOR_IT);
                                                digitando=false;
                                            }

                                            if(event.type==ALLEGRO_EVENT_KEY_CHAR){
                                                if(event.keyboard.keycode!=ALLEGRO_KEY_BACKSPACE){
                                                    ip[iplen]=(char)event.keyboard.unichar;
                                                    iplen++;
                                                    ip[iplen]='\0';
                                                }
                                                else{
                                                    if(iplen!=0){
                                                        iplen--;
                                                    }
                                                    ip[iplen]='\0';
                                                }
                                            }
                                            if(event.type==ALLEGRO_EVENT_TIMER){
                                                al_draw_bitmap(fundo,0,0,0);
                                                al_draw_text(fontetitulo,al_map_rgb(150,150,150),(width/2)-(float) al_get_text_width(fontetitulo,"Digite o IP")/2,100,0,"Digite o IP");
                                                al_draw_filled_rectangle((width/2)-155,190,(width/2)+165,230,al_map_rgb(100,100,100));
                                                al_draw_textf(fonte,al_map_rgb(255,0,255),(width/2)-155,190,0,"%s",ip);
                                                if(id==0){
                                                    al_draw_text(fontetitulo,al_map_rgb(255,0,0),(width/2)-70,600,0,"Jogar");
                                                    al_draw_filled_rectangle((width/2)-32,700,(width/2)+32,764,al_map_rgb(200,100,100));
                                                }
                                                al_draw_bitmap(cursor,x,y,0);
                                                al_flip_display();
                                            }
                                        }
                                    }
                                }

                                //se o cara clickar no jogar e for o host
                                if(x>=(width/2)-32&&x<=(width/2)+32&&y>=700&&y<=764){
                                    if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                                        msg=CLIENTSTART;
                                        sendMsgToServer(&msg,sizeof(int));
                                    }
                                } 

                                if(id!=DESCONECTADO){
                                    status=recvMsgFromServer(&flag,DONT_WAIT);

                                    if(flag==STARTGAME){
                                        jogo=true;
                                    }
                                }

                                if(jogo){
                                    recvMsgFromServer(&(jogador[0]),WAIT_FOR_IT);
                                    recvMsgFromServer(&(jogador[1]),WAIT_FOR_IT);
                                    recvMsgFromServer(&(jogador[2]),WAIT_FOR_IT);
                                    recvMsgFromServer(&(jogador[3]),WAIT_FOR_IT);
                                    al_destroy_event_queue(queueLogin);
                                    queueJogo=al_create_event_queue();
                                    al_register_event_source(queueJogo,al_get_display_event_source(display));
                                    al_register_event_source(queueJogo,al_get_keyboard_event_source());
                                    al_register_event_source(queueJogo,al_get_timer_event_source(timer));
                                    int cameraPosition[2]={0,0};
                                    bool w=false,a=false,s=false,d=false,f=false,iv=false;
                                    while(jogo){
                                            
                                        al_wait_for_event(queueJogo,&event);

                                        
                                        if(event.keyboard.keycode==ALLEGRO_KEY_ESCAPE){
                                            jogo=false;
                                        }

                                        if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
                                            run=false;
                                            login=false;
                                            chat=false;
                                            jogo=false;
                                        }
                                        //recebe a msg do servidor
                                        status=recvMsgFromServer(&mServer,DONT_WAIT);
                                        //disconnect
                                        if(status==SERVER_DISCONNECTED){
                                            jogo=false;
                                            chat=false;
                                            exit(0);
                                        }
                                        else if(status!=NO_MESSAGE){
                                            //morreu
                                            if(mServer.flag==2){
                                                while(jogo){
                                                    status=recvMsgFromServer(&mServer,DONT_WAIT);
                                                    //disconnect
                                                    if(status==SERVER_DISCONNECTED){
                                                        jogo=false;
                                                        chat=false;
                                                        exit (0);
                                                    }
                                                    //normal
                                                    if(mServer.flag==1&&status!=NO_MESSAGE){
                                                        jogador[0]=mServer.jogadores[0];
                                                        jogador[1]=mServer.jogadores[1];
                                                        jogador[2]=mServer.jogadores[2];
                                                        jogador[3]=mServer.jogadores[3];
                                                    }
                                                        
                                                    al_wait_for_event(queueJogo,&event);

                                                    if(event.keyboard.keycode==ALLEGRO_KEY_ESCAPE){
                                                        jogo=false;
                                                    }

                                                    if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
                                                        run=false;
                                                        login=false;
                                                        chat=false;
                                                        jogo=false;
                                                    }
                                                    //event timer morte
                                                    if(event.type==ALLEGRO_EVENT_TIMER){
                                                        if(id==0){
                                                            msg=31;
                                                            sendMsgToServer(&msg,sizeof(int));
                                                        }                        
                                                        al_draw_bitmap(fundo,0,0,0);
                                                        int i,k;
                                                        for(i=0;i<4;i++){
                                                            al_draw_textf(fontemenor,al_map_rgb(255,0,255),64*jogador[i].x+((float)al_get_bitmap_width(personagens[jogador[i].personagem])/20)-((float) al_get_text_width(fontemenor,jogador[i].nome))/2,64*jogador[i].y-30,0,"%s",jogador[i].nome);
                                                            al_draw_scaled_bitmap(personagens[jogador[i].personagem],(float)jogador[i].persoX*32,(float)jogador[i].persoY*32,32,32,jogador[i].x*64+jogador[i].flip*64,jogador[i].y*64,64,64,jogador[i].flip);
                                                            for(k=0;k<3;k++){
                                                                if(jogador[i].castelos[k].vida==0){
                                                                    flag=1;
                                                                }
                                                            }
                                                            al_draw_text(fontetitulo,al_map_rgb(0,0,0), width/2,height/2,0,"VOCE PERDEU");
                                                        }
                                                        al_flip_display();
                                                    } 
                                                }
                                                
                                            }
                                            //ganhou
                                            else if(mServer.flag==3){
                                                al_draw_text(fontetitulo,al_map_rgb(0,0,0), width/2,height/2,0,"VOCE GANHOU!!!");
                                                al_flip_display();
                                                al_rest(10);
                                                chat=false;
                                                jogo=false;
                                            }
                                            //normal
                                            else if(mServer.flag==1){
                                                jogador[0]=mServer.jogadores[0];
                                                jogador[1]=mServer.jogadores[1];
                                                jogador[2]=mServer.jogadores[2];
                                                jogador[3]=mServer.jogadores[3];
                                            }
                                        }
                                            if(jogador[id].respawn==0){

                                                if(event.type==ALLEGRO_EVENT_KEY_UP){
                                                    w=false;
                                                    iv=false;
                                                }
                                                //cima
                                                if(event.keyboard.keycode==ALLEGRO_KEY_UP&&!w){
                                                    msg=CIMA;
                                                    sendMsgToServer(&msg,sizeof(int));
                                                    w=true;
                                                    puts("cima");
                                                }
                                                //baixo
                                                if(event.keyboard.keycode==ALLEGRO_KEY_DOWN&&!w){
                                                    msg=BAIXO;
                                                    sendMsgToServer(&msg,sizeof(int));
                                                    w=true;
                                                    puts("baixo");
                                                }
                                                //direita
                                                if(event.keyboard.keycode==ALLEGRO_KEY_RIGHT&&!w){
                                                    msg=DIREITA;
                                                    sendMsgToServer(&msg,sizeof(int));
                                                    w=true;
                                                    puts("direita");
                                                }
                                                //esquerda
                                                if(event.keyboard.keycode==ALLEGRO_KEY_LEFT&&!w){
                                                    msg=ESQUERDA;
                                                    sendMsgToServer(&msg,sizeof(int));
                                                    w=true;
                                                    puts("esquerda");
                                                }
                                                //inventario
                                                if(event.keyboard.keycode==ALLEGRO_KEY_I&&!iv){
                                                    al_destroy_event_queue(queueJogo);
                                                    queueInventario=al_create_event_queue();
                                                    al_register_event_source(queueInventario,al_get_mouse_event_source());
                                                    al_register_event_source(queueInventario,al_get_keyboard_event_source());
                                                    al_register_event_source(queueInventario,al_get_display_event_source(display));
                                                    al_register_event_source(queueInventario,al_get_timer_event_source(timer));
                                                    al_rest(0.1);
                                                    al_flush_event_queue(queueInventario);
                                                    bool inventario=true;
                                                    iv=true;
                                                    while(inventario){
                                                        
                                                        status=recvMsgFromServer(&mServer,DONT_WAIT);
                                                        //disconnect
                                                        if(status==SERVER_DISCONNECTED){
                                                            puts("ahaaaaaa");
                                                            jogo=false;
                                                            chat=false;
                                                            exit(0);
                                                        }
                                                        else if(status!=NO_MESSAGE){
                                                            //morreu
                                                            if(mServer.flag==2){
                                                                puts("oush");
                                                                while(jogo){
                                                                    status=recvMsgFromServer(&mServer,DONT_WAIT);
                                                                    //disconnect
                                                                    if(status==SERVER_DISCONNECTED){
                                                                        jogo=false;
                                                                        chat=false;
                                                                        inventario=false;
                                                                        exit (0);
                                                                    }
                                                                    //normal
                                                                    if(mServer.flag==1&&status!=NO_MESSAGE){
                                                                        jogador[0]=mServer.jogadores[0];
                                                                        jogador[1]=mServer.jogadores[1];
                                                                        jogador[2]=mServer.jogadores[2];
                                                                        jogador[3]=mServer.jogadores[3];
                                                                    }
                                                                        
                                                                    al_wait_for_event(queueJogo,&event);

                                                                    if(event.keyboard.keycode==ALLEGRO_KEY_ESCAPE){
                                                                        jogo=false;
                                                                        inventario=false;
                                                                    }

                                                                    if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
                                                                        run=false;
                                                                        login=false;
                                                                        chat=false;
                                                                        jogo=false;
                                                                        inventario=false;
                                                                    }
                                                                    //event timer morte
                                                                    if(event.type==ALLEGRO_EVENT_TIMER){
                                                                        if(id==0){
                                                                            msg=31;
                                                                            sendMsgToServer(&msg,sizeof(int));
                                                                        }                        
                                                                        al_draw_bitmap(fundo,0,0,0);
                                                                        int i,k;
                                                                        for(i=0;i<4;i++){
                                                                            al_draw_textf(fontemenor,al_map_rgb(255,0,255),64*jogador[i].x+((float)al_get_bitmap_width(personagens[jogador[i].personagem])/20)-((float) al_get_text_width(fontemenor,jogador[i].nome))/2,64*jogador[i].y-30,0,"%s",jogador[i].nome);
                                                                            al_draw_scaled_bitmap(personagens[jogador[i].personagem],(float)jogador[i].persoX*32,(float)jogador[i].persoY*32,32,32,jogador[i].x*64+jogador[i].flip*64,jogador[i].y*64,64,64,jogador[i].flip);
                                                                            for(k=0;k<3;k++){
                                                                                if(jogador[i].castelos[k].vida==0){
                                                                                    flag=1;
                                                                                }
                                                                            }
                                                                            al_draw_text(fontetitulo,al_map_rgb(0,0,0), width/2,height/2,0,"VOCE PERDEU");
                                                                        }
                                                                        al_flip_display();
                                                                    } 
                                                                }
                                                                
                                                            }
                                                            //ganhou
                                                            else if(mServer.flag==3){
                                                                al_draw_text(fontetitulo,al_map_rgb(0,0,0), width/2,height/2,0,"VOCE GANHOU!!!");
                                                                al_flip_display();
                                                                al_rest(10);
                                                                chat=false;
                                                                jogo=false;
                                                            }
                                                            //normal
                                                            else if(mServer.flag==1){
                                                                jogador[0]=mServer.jogadores[0];
                                                                jogador[1]=mServer.jogadores[1];
                                                                jogador[2]=mServer.jogadores[2];
                                                                jogador[3]=mServer.jogadores[3];
                                                            }
                                                        }

                                                        al_wait_for_event(queueInventario,&event);
                                                        if(event.type==ALLEGRO_EVENT_MOUSE_AXES){
                                                            x=(float)event.mouse.x;
                                                            y=(float)event.mouse.y;
                                                        }

                                                        if(event.type==ALLEGRO_EVENT_KEY_UP){
                                                            iv=false;
                                                        }

                                                        if((event.keyboard.keycode==ALLEGRO_KEY_I&&!i)||event.keyboard.keycode==ALLEGRO_KEY_ESCAPE){
                                                            inventario=false;
                                                            iv=true;
                                                        }

                                                        if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
                                                            run=false;
                                                            login=false;
                                                            chat=false;
                                                            jogo=false;
                                                            inventario=false;
                                                        }

                                                        if(x>=(width/20)+(((width*3/20)-67))/2&&x<=(width/20)+(((width*3/20)-67))/2+67&&y>=(height/20)+128&&y<=(height/20)+128+67){
                                                            if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                                                                msg=UPCIDADE;
                                                                sendMsgToServer(&msg,sizeof(int));
                                                                
                                                            }
                                                        }

                                                        if(x>=width*4/20+(((width*3/20)-67))/2&&x<=(width*4/20)+((((width*3/20)-67))/2)+67&&y>=(height/20)+128&&y<=(height/20)+128+67){
                                                                if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                                                                    msg=UPQUARTEL;
                                                                    sendMsgToServer(&msg,sizeof(int));
                                                                }
                                                            }

                                                        if(x>=(width*7/20)+((((width*3/20)-67))/2)&&x<=(width*7/20)+((((width*3/20)-67))/2)+67&&y>=(height/20)+128&&y<=(height/20)+128+67){
                                                            if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                                                                msg=UPMINA;
                                                                sendMsgToServer(&msg,sizeof(int));
                                                            }
                                                        }

                                                        if(x>=(width/20)+(((width*3/20)-67))/2&&x<=(width/20)+(((width*3/20)-67))/2+67&&y>=(height/20)+228&&y<=(height/20)+228+67){
                                                            if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                                                                msg=UPMURALHA;
                                                                sendMsgToServer(&msg,sizeof(int));
                                                            }
                                                        }

                                                        

                                                        //timer inventario
                                                        if(event.type==ALLEGRO_EVENT_TIMER){

                                                            if(id==0){
                                                                msg=31;
                                                                sendMsgToServer(&msg,sizeof(int));
                                                            }
                                                            cameraUpdate(cameraPosition,jogador[id].x,jogador[id].y,64,64);
                                                            al_identity_transform(&camera);
                                                            al_translate_transform(&camera,-(float)cameraPosition[0],-(float)cameraPosition[1]);
                                                            al_use_transform(&camera);

                                                            al_draw_bitmap(fundo,0,0,0);
                                                            for(i=0;i<4;i++){
                                                                al_draw_textf(fontemenor,al_map_rgb(255,0,255),64*jogador[i].x+((float)al_get_bitmap_width(personagens[jogador[i].personagem])/20)-((float) al_get_text_width(fontemenor,jogador[i].nome))/2,64*jogador[i].y-30,0,"%s",jogador[i].nome);
                                                                al_draw_scaled_bitmap(personagens[jogador[i].personagem],(float)jogador[i].persoX*32,(float)jogador[i].persoY*32,32,32,jogador[i].x*64+jogador[i].flip*64,jogador[i].y*64,64,64,jogador[i].flip);
                                                                for(k=0;k<3;k++){
                                                                    if(jogador[i].castelos[k].vida==0){
                                                                        //printa no lugar um castelo destruido
                                                                    }
                                                                }
                                                            }

                                                            al_identity_transform(&camera);
                                                            al_translate_transform(&camera,0,0);
                                                            al_use_transform(&camera);

                                                            al_draw_filled_rectangle(30,10,10+300*(jogador[id].health/(float)jogador[id].healthM),25,al_map_rgb(0,255,0));
                                                            al_draw_filled_rectangle(10+300*(jogador[id].health/(float)jogador[id].healthM),10,300+10,25,al_map_rgb(255,0,0));
                                                            al_draw_filled_rectangle(width/20,height/20,width*13/20,height*13/20,al_map_rgb(139,69,19));
                                                            al_draw_scaled_bitmap(bau,0,0,48,48,120,10,80,80,0);
                                                            al_draw_textf(fonte,al_map_rgb(0,0,0), 170,43,0,"%.0lf",jogador[id].dinheiro);
                                                            if(jogador[id].up.cidade<2){
                                                                al_draw_bitmap(upgradeBits[0][jogador[id].up.cidade],(width/20)+(((width*3/20)-67))/2,(height/20)+128,0);
                                                                if(x>=(width/20)+(((width*3/20)-67))/2&&x<=(width/20)+(((width*3/20)-67))/2+67&&y>=(height/20)+128&&y<=(height/20)+128+67){
                                                                    al_draw_filled_rectangle(x+32,y-10,x+400,y+20,al_map_rgb(255,248,220));
                                                                    al_draw_textf(fonteupgrades,al_map_rgb(0,0,0),x+52,y-5,0,"Castelos tem mais vida e geram mais dinheiro. CUSTO:%d",(jogador[id].up.cidade+1)*500);
                                                                }
                                                            }
                                                            if(jogador[id].up.quartel<2){
                                                                al_draw_bitmap(upgradeBits[1][jogador[id].up.quartel],(width*4/20)+((((width*3/20)-67))/2),(height/20)+128,0);
                                                                if(x>=width*4/20+(((width*3/20)-67))/2&&x<=(width*4/20)+((((width*3/20)-67))/2)+67&&y>=(height/20)+128&&y<=(height/20)+128+67){
                                                                    al_draw_filled_rectangle(x+32,y-10,x+400,y+20,al_map_rgb(255,248,220));
                                                                    al_draw_textf(fonteupgrades,al_map_rgb(0,0,0),x+52,y-5,0,"Melhora suas unidades e desbloqueia unidades novas. CUSTO:%d",(jogador[id].up.quartel+1)*300);
                                                                }
                                                            }
                                                            if(jogador[id].up.mina<2){
                                                                al_draw_bitmap(upgradeBits[2][jogador[id].up.mina],(width*7/20)+((((width*3/20)-67))/2),(height/20)+128,0);
                                                                if(x>=(width*7/20)+((((width*3/20)-67))/2)&&x<=(width*7/20)+((((width*3/20)-67))/2)+67&&y>=(height/20)+128&&y<=(height/20)+128+67){
                                                                    al_draw_filled_rectangle(x+32,y-10,x+400,y+20,al_map_rgb(255,248,220));
                                                                    al_draw_textf(fonteupgrades,al_map_rgb(0,0,0),x+52,y-5,0,"Castelos geram mais dinheiro. CUSTO:%d",(jogador[id].up.mina+1)*400);
                                                                }
                                                            }
                                                            if(jogador[id].up.muralha<2){
                                                                al_draw_bitmap(upgradeBits[3][jogador[id].up.muralha],(width/20)+(((width*3/20)-67))/2,(height/20)+228,0);
                                                                if(x>=(width/20)+(((width*3/20)-67))/2&&x<=(width/20)+(((width*3/20)-67))/2+67&&y>=(height/20)+228&&y<=(height/20)+228+67){
                                                                    al_draw_filled_rectangle(x+32,y-10,x+400,y+20,al_map_rgb(255,248,220));
                                                                    al_draw_textf(fonteupgrades,al_map_rgb(0,0,0),x+52,y-5,0,"Castelos tem mais vida. CUSTO:%d",(jogador[id].up.muralha+1)*300);
                                                                }
                                                            }
                                                            al_draw_bitmap(cursor,(float)x,(float)y,0);
                                                            al_flip_display();
                                                        }


                                                    }
                                                    //create destroy inv->jogo
                                                    if(1){
                                                        al_destroy_event_queue(queueInventario);
                                                        queueJogo=al_create_event_queue();
                                                        al_register_event_source(queueJogo,al_get_display_event_source(display));
                                                        al_register_event_source(queueJogo,al_get_keyboard_event_source());
                                                        al_register_event_source(queueJogo,al_get_timer_event_source(timer));
                                                        al_rest(0.1);
                                                        al_flush_event_queue(queueJogo);
                                                    }

                                                    
                                                }

                                                //ataque
                                                if(event.keyboard.keycode==ALLEGRO_KEY_F&&!w){
                                                    msg=ATAQUE;
                                                    sendMsgToServer(&msg,sizeof(int));
                                                    f=true;
                                                }
                                            }
                                            //event timer jogo
                                            if(event.type==ALLEGRO_EVENT_TIMER){
                                                if(id==0){
                                                    msg=TICKTIMER;
                                                    sendMsgToServer(&msg,sizeof(int));
                                                }

                                                cameraUpdate(cameraPosition,jogador[id].x,jogador[id].y,64,64);
                                                al_identity_transform(&camera);
                                                al_translate_transform(&camera,-(float)cameraPosition[0],-(float)cameraPosition[1]);
                                                al_use_transform(&camera);
                                                al_clear_to_color(al_map_rgb(0,0,0));
                                                al_draw_bitmap(mapa,0,0,0);
                                                for(i=0;i<4;i++){
                                                    al_draw_textf(fontemenor,al_map_rgb(255,0,255),64*jogador[i].x+((float)al_get_bitmap_width(personagens[jogador[i].personagem])/20)-((float) al_get_text_width(fontemenor,jogador[i].nome))/2,64*jogador[i].y-30,0,"%s",jogador[i].nome);
                                                    al_draw_scaled_bitmap(personagens[jogador[i].personagem],(float)jogador[i].persoX*32,(float)jogador[i].persoY*32,32,32,jogador[i].x*64,jogador[i].y*64,64,64,jogador[i].flip);
                                                    for(k=0;k<3;k++){
                                                        if(jogador[i].castelos[k].vida==0){

                                                        }
                                                    }
                                                }
                                                al_identity_transform(&camera);
                                                al_translate_transform(&camera,0,0);
                                                al_use_transform(&camera);

                                                al_draw_filled_rectangle(30,10,10+300*(jogador[id].health/(float)jogador[id].healthM),25,al_map_rgb(0,255,0));
                                                al_draw_filled_rectangle(10+300*(jogador[id].health/(float)jogador[id].healthM),10,300+10,25,al_map_rgb(255,0,0));
                                                al_draw_scaled_bitmap(bau,0,0,48,48,30,15,60,60,0);
                                                al_draw_textf(fontemenor,al_map_rgb(0,0,0), 70,43,0,"%.0lf",jogador[id].dinheiro);
                                                al_flip_display();
                                            } 

                                        }
                                        //create destroy
                                        if(1){
                                            al_destroy_event_queue(queueJogo);
                                            queueLogin=al_create_event_queue();
                                            al_register_event_source(queueLogin,al_get_mouse_event_source());
                                            al_register_event_source(queueLogin,al_get_display_event_source(display));
                                            al_register_event_source(queueLogin,al_get_keyboard_event_source());
                                            al_register_event_source(queueLogin,al_get_timer_event_source(timer));
                                            al_rest(0.1);
                                            al_flush_event_queue(queueLogin);
                                        }
                                }

                               

                                //event timer chat
                                if(event.type==ALLEGRO_EVENT_TIMER){
                                    al_draw_bitmap(fundo,0,0,0);
                                    al_draw_text(fontetitulo,al_map_rgb(150,150,150),(width/2)-(float) al_get_text_width(fontetitulo,"Digite o IP")/2,100,0,"Digite o IP");
                                    al_draw_filled_rectangle((width/2)-155,190,(width/2)+165,230,al_map_rgb(100,100,100));
                                    al_draw_textf(fonte,al_map_rgb(255,0,255),(width/2)-155,190,0,"%s",ip);
                                    al_draw_text(fontetitulo,al_map_rgb(255,0,0),(width/2)-70,600,0,"Jogar");
                                    al_draw_filled_rectangle((width/2)-32,700,(width/2)+32,764,al_map_rgb(200,100,100));
                                    al_draw_bitmap(cursor,(float)x,(float)y,0);
                                    al_flip_display();
                                } 
                            }
                            al_rest(0.1);
                            id=50;
                            al_flush_event_queue(queueLogin);
                        }
                    }
                    //event timer login
                    if(event.type==ALLEGRO_EVENT_TIMER){
                        al_draw_bitmap(fundo,0,0,0);
                        al_draw_text(fontetitulo,al_map_rgb(150,150,150),(width/2)-(float) al_get_text_width(fontetitulo,"Digite seu nome")/2,100,0,"Digite seu nome");
                        al_draw_text(fontetitulo,al_map_rgb(255,0,0),(width/2)-(float) al_get_text_width(fontetitulo,"Jogar")/2,600,0,"Jogar");
                        al_draw_text(fontetitulo,al_map_rgb(255,0,0),(width/2)-(float) al_get_text_width(fontetitulo,"Escolha seu personagem")/2,250,0,"Escolha seu personagem");
                        al_draw_scaled_bitmap(personagens[0],0,0,32,32,(width/4)-((float)sizep[0]/2),350,(float)sizep[0],(float)sizep[0],0);
                        al_draw_scaled_bitmap(personagens[1],0,0,32,32,(width/2)-((float)sizep[1]/2),350,(float)sizep[1],(float)sizep[1],0);
                        al_draw_scaled_bitmap(personagens[2],0,0,32,32,(width*3/4)-((float)sizep[2]/2),350,(float)sizep[2],(float)sizep[2],0);
                        al_draw_filled_rectangle((width/2)-32,700,(width/2)+32,764,al_map_rgb(200,100,100));
                        al_draw_filled_rectangle((width/2)-155,190,(width/2)+165,230,al_map_rgb(100,100,100));
                        al_draw_textf(fonte,al_map_rgb(255,0,255),(width/2)-155,190,0,"%s",nomeJogador);
                        al_draw_bitmap(cursor,(float)x,(float)y,0);
                        al_flip_display();
                    }           
                }
                //create destroy
                if(1){
                    al_destroy_event_queue(queueLogin);
                    queue=al_create_event_queue();
                    al_register_event_source(queue,al_get_mouse_event_source());
                    al_register_event_source(queue,al_get_display_event_source(display));
                    al_register_event_source(queue,al_get_keyboard_event_source());
                    al_register_event_source(queue,al_get_timer_event_source(timer));
                    al_rest(0.1);
                    al_flush_event_queue(queue);
                }

            }
        }

        //tutorial
        if(x>=630&&x<=820&&y>=400&&y<=450){
            fontetutorial=fontemaior;
            if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                tutorial=true;
                al_destroy_event_queue(queue);
                queueTutorial=al_create_event_queue();
                al_register_event_source(queueTutorial,al_get_mouse_event_source());
                al_register_event_source(queueTutorial,al_get_display_event_source(display));
                al_register_event_source(queueTutorial,al_get_keyboard_event_source());
                al_register_event_source(queueTutorial,al_get_timer_event_source(timer));
                while(tutorial){
                    al_wait_for_event(queueTutorial,&event);
                    if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
                        tutorial=false;
                        run=false;
                    }

                    if(event.keyboard.keycode==ALLEGRO_KEY_ESCAPE){
                        tutorial=false;
                    }

                    if(event.type==ALLEGRO_EVENT_MOUSE_AXES){
                        x=(float)event.mouse.x;
                        y=(float)event.mouse.y;
                    }  

                    if(event.type==ALLEGRO_EVENT_TIMER){
                        al_draw_bitmap(fundo,0,0,0);
                        al_draw_filled_rectangle((width)/2-400,50,(width)/2+400,700,al_map_rgb(150,40,40));
                        al_draw_text(fontetitulo,al_map_rgb(150,150,150),(width/2)-85,40,0,"Tutorial");
                        al_draw_bitmap(cursor,x,y,0);
                        al_flip_display();
                    }
                }
                al_destroy_event_queue(queueTutorial);
                queue=al_create_event_queue();
                al_register_event_source(queue,al_get_mouse_event_source());
                al_register_event_source(queue,al_get_display_event_source(display));
                al_register_event_source(queue,al_get_keyboard_event_source());
                al_register_event_source(queue,al_get_timer_event_source(timer));
                al_rest(0.1);
                al_flush_event_queue(queue);
            }
        }
        //sair
        if(x>=680&&x<=780&&y>=500&&y<=550){
            fontesair=fontemaior;
            if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                run=false;
            }
        }


        if(event.type==ALLEGRO_EVENT_TIMER){
            al_draw_bitmap(fundo,0,0,0);
            al_draw_text(fontejogar,al_map_rgb(150,150,150),width/2-(float)(float) al_get_text_width(fontejogar,"JOGAR")/2,300,0,"JOGAR");
            al_draw_text(fontetutorial,al_map_rgb(150,150,150),width/2-(float)(float) al_get_text_width(fontetutorial,"TUTORIAL")/2,400,0,"TUTORIAL");
            al_draw_text(fontesair,al_map_rgb(150,150,150),width/2-(float)(float) al_get_text_width(fontesair,"SAIR")/2,500,0,"SAIR");
            al_draw_bitmap(cursor,(float)x,(float)y,0);
            al_flip_display();
        }
        
    }

    for(i=0;i<4;i++){
        free(upgradeBits[i]);
    }
    free(upgradeBits);
    free(personagens);
    al_destroy_display(display);
    al_destroy_event_queue(queue);
    return 0;
}