#include <Imgn.hpp>

namespace Imgn
{
    class EditorLayer : public Layer
    {


    public:
        EditorLayer() /*Constructor*/
        {
        }

        ~EditorLayer() /*Destructor*/
        {
        }

        /*Copy Constructor*/
        EditorLayer(const EditorLayer& pOther) = default;

        /*Copy Assignment Operator*/
        EditorLayer& operator=(const EditorLayer& pOther) = default;

        /*Move Constructor*/
        EditorLayer(EditorLayer&& pOther) noexcept = default;

        /*Move Assignment Operator*/
        EditorLayer& operator=(EditorLayer&& pOther) noexcept = default;

        /*Class Functions*/
        virtual void Sleep() override;
        virtual void WakeUp() override;
        virtual void OnImGuiRender() override;
        virtual void Dream(Time pTime) override;
        virtual void OnEvent(Event& pEvent) override;

    };
}