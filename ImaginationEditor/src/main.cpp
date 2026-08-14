#include <Imgn.hpp>

#include "EditorLayer.hpp"

namespace Imgn
{
    class ImaginationEditor : public ImgnApp
    {


    public:
        ImaginationEditor() /*Constructor*/
        {
        }

        ~ImaginationEditor() /*Destructor*/
        {
        }

        /*Copy Constructor*/
        ImaginationEditor(const ImaginationEditor& pOther) = default;

        /*Copy Assignment Operator*/
        ImaginationEditor& operator=(const ImaginationEditor& pOther) = default;

        /*Move Constructor*/
        ImaginationEditor(ImaginationEditor&& pOther) noexcept = default;

        /*Move Assignment Operator*/
        ImaginationEditor& operator=(ImaginationEditor&& pOther) noexcept = default;

        /*Class Functions*/
    };

    ImgnApp* CreateApplication()
    {
        return new ImaginationEditor();
    }
}