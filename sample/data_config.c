/*-----------------data config start  -------------------*/ 

#define TOTAL_PROPERTY_COUNT 5

static sDataPoint    sg_DataTemplate[TOTAL_PROPERTY_COUNT];

typedef struct _ProductDataDefine {
    TYPE_DEF_TEMPLATE_BOOL m_light_switch;
    TYPE_DEF_TEMPLATE_ENUM m_color;
    TYPE_DEF_TEMPLATE_FLOAT m_brightness;
    TYPE_DEF_TEMPLATE_INT m_light_id;
    TYPE_DEF_TEMPLATE_STRING m_name[64+1];
} ProductDataDefine;

static   ProductDataDefine     sg_ProductData;

static void _init_data_template(void)
{
    memset((void *) & sg_ProductData, 0, sizeof(ProductDataDefine));
    sg_DataTemplate[0].data_property.key  = "light_switch";
    sg_DataTemplate[0].data_property.data = &sg_ProductData.m_light_switch;
    sg_DataTemplate[0].data_property.type = TYPE_TEMPLATE_BOOL;

    sg_DataTemplate[1].data_property.key  = "color";
    sg_DataTemplate[1].data_property.data = &sg_ProductData.m_color;
    sg_DataTemplate[1].data_property.type = TYPE_TEMPLATEENUM;

    sg_DataTemplate[2].data_property.key  = "brightness";
    sg_DataTemplate[2].data_property.data = &sg_ProductData.m_brightness;
    sg_DataTemplate[2].data_property.type = TYPE_TEMPLATE_FLOAT;

    sg_DataTemplate[3].data_property.key  = "light_id";
    sg_DataTemplate[3].data_property.data = &sg_ProductData.m_light_id;
    sg_DataTemplate[3].data_property.type = TYPE_TEMPLATE_INT;

    sg_DataTemplate[4].data_property.key  = "name";
    sg_DataTemplate[4].data_property.data = sg_ProductData.m_name;
    sg_DataTemplate[4].data_property.type = TYPE_TEMPLATE_STRING;

};
