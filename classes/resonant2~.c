// Porres 2017

#include "m_pd.h"
#include <math.h>

#define PI M_PI

typedef struct _resonant2 {
    t_object    x_obj;
    t_int       x_n;
    t_inlet    *x_inlet_freq;
    t_inlet    *x_inlet_t1;
    t_inlet    *x_inlet_t2;
    t_outlet   *x_out;
    t_float     x_nyq;
    double  x_xnm1;
    double  x_xnm2;
    double  x_ynm1;
    double  x_ynm2;
    } t_resonant2;

static t_class *resonant2_class;

static t_int *resonant2_perform(t_int *w){
    t_resonant2 *x = (t_resonant2 *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *in4 = (t_float *)(w[6]);
    t_float *out = (t_float *)(w[7]);
    double xnm1 = x->x_xnm1;
    double xnm2 = x->x_xnm2;
    double ynm1 = x->x_ynm1;
    double ynm2 = x->x_ynm2;
    t_float nyq = x->x_nyq;
    while (nblock--){
        double xn = *in1++, f = *in2++, t1 = *in3++, t2 = *in4++;
        double q, omega, alphaQ, cos_w, a0, a2, b0, b1, b2, yn;
        if (f < 0.000001)
            f = 0.000001;
        if (f > nyq - 0.000001)
            f = nyq - 0.000001;
        if (t1 <= 0)
            *out++ = xn;
        else{
            q = f * (PI * t1/1000) / log(1000); // t60
            omega = f * PI/nyq;
            alphaQ = sin(omega) / (2*q);
            cos_w = cos(omega);
            b0 = alphaQ + 1;
            a0 = alphaQ*q / b0;
            a2 = -a0;
            b1 = 2*cos_w / b0;
            b2 = (alphaQ - 1) / b0;
            yn = a0 * xn + a2 * xnm2 + b1 * ynm1 + b2 * ynm2;
            *out++ = yn;
            }
        xnm2 = xnm1;
        xnm1 = xn;
        ynm2 = ynm1;
        ynm1 = yn;
    }
    x->x_xnm1 = xnm1;
    x->x_xnm2 = xnm2;
    x->x_ynm1 = ynm1;
    x->x_ynm2 = ynm2;
    return (w + 8);
}

static void resonant2_dsp(t_resonant2 *x, t_signal **sp){
    x->x_nyq = sp[0]->s_sr / 2;
    dsp_add(resonant2_perform, 7, x, sp[0]->s_n, sp[0]->s_vec,
        sp[1]->s_vec, sp[2]->s_vec,sp[3]->s_vec);
}

static void resonant2_clear(t_resonant2 *x){
    x->x_xnm1 = x->x_xnm2 = x->x_ynm1 = x->x_ynm2 = 0.;
}

static void *resonant2_new(t_symbol *s, int argc, t_atom *argv){
    t_resonant2 *x = (t_resonant2 *)pd_new(resonant2_class);
    float freq = 0;
    float reson1 = 0;
    float reson2 = 0;
/////////////////////////////////////////////////////////////////////////////////////
    int argnum = 0;
    while(argc > 0)
    {
        if(argv -> a_type == A_FLOAT){ //if current argument is a float
            t_float argval = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    freq = argval;
                    break;
                case 1:
                    reson1 = argval;
                    break;
                case 2:
                    reson2 = argval;
                    break;
                default:
                    break;
            };
            argnum++;
            argc--;
            argv++;
        }
        else
            goto errstate;
    };
/////////////////////////////////////////////////////////////////////////////////////

    x->x_inlet_freq = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
        pd_float((t_pd *)x->x_inlet_freq, freq);
    x->x_inlet_t1 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
        pd_float((t_pd *)x->x_inlet_t1, reson1);
    x->x_inlet_t2 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
        pd_float((t_pd *)x->x_inlet_t2, reson2);
    x->x_out = outlet_new((t_object *)x, &s_signal);
    
    return (x);
errstate:
    pd_error(x, "resonant2~: improper args");
    return NULL;
}

void resonant2_tilde_setup(void)
{
    resonant2_class = class_new(gensym("resonant2~"), (t_newmethod)resonant2_new, 0,
        sizeof(t_resonant2), CLASS_DEFAULT, A_GIMME, 0);
    class_addmethod(resonant2_class, (t_method)resonant2_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(resonant2_class, nullfn, gensym("signal"), 0);
    class_addmethod(resonant2_class, (t_method)resonant2_clear, gensym("clear"), 0);
}
