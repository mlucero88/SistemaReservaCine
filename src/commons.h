#ifndef SISTEMARESERVACINE_COMMONS_H
#define SISTEMARESERVACINE_COMMONS_H

#define Q_CINE_CLI 1
#define Q_CLI_CINE 2
#define Q_ADMIN_CINE 3
#define Q_CINE_ADMIN 4

#define MAX_SALAS 100
#define MAX_ASIENTOS 300

#define LOGIN 1

enum op_type {LOGIN,RESULT,PEDIR_INFO_SALAS,INFO_SALAS,ELEGIR_SALA,INFO_ASIENTOS,ELEGIR_ASIENTOS,CONFIRMACION_RESERVA,PAGO};

struct msg {
    long mtype;
    enum op_type type;
    union {
        struct {
            int id;
        } login;
        struct {
            bool ok;
        } result;
        struct {
        } pedir_info_salas;
        struct {
            int salas [MAX_SALAS];
            int len; // cantidad de salas que hay
        } info_salas;
        struct {
            int nro_sala;
        } elegir_sala;
        struct {
            bool asientos [MAX_ASIENTOS];
            int len;
        } info_asientos;
        struct {
            int asientos [MAX_RESERVAS];
            int len;
        } elegir_asientos;
        struct {
            int asientos [MAX_RESERVAS];
            int len;
        } confirmacion_reserva;
        struct {
            int plata;
        } pago;
    } op;
};


#endif //SISTEMARESERVACINE_COMMONS_H
